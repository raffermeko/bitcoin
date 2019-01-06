// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdexcept>

#include <flatfile.h>
#include <logging.h>
#include <tinyformat.h>
#include <util/system.h>

FlatFileSeq::FlatFileSeq(fs::path dir, const char* prefix, size_t chunk_size) :
    m_dir(std::move(dir)),
    m_prefix(prefix),
    m_chunk_size(chunk_size)
{
    if (chunk_size == 0) {
        throw std::invalid_argument("chunk_size must be positive");
    }
}

fs::path FlatFileSeq::FileName(const CDiskBlockPos& pos) const
{
    return m_dir / strprintf("%s%05u.dat", m_prefix, pos.nFile);
}

FILE* FlatFileSeq::Open(const CDiskBlockPos& pos, bool fReadOnly)
{
    if (pos.IsNull())
        return nullptr;
    fs::path path = FileName(pos);
    fs::create_directories(path.parent_path());
    FILE* file = fsbridge::fopen(path, fReadOnly ? "rb": "rb+");
    if (!file && !fReadOnly)
        file = fsbridge::fopen(path, "wb+");
    if (!file) {
        LogPrintf("Unable to open file %s\n", path.string());
        return nullptr;
    }
    if (pos.nPos) {
        if (fseek(file, pos.nPos, SEEK_SET)) {
            LogPrintf("Unable to seek to position %u of %s\n", pos.nPos, path.string());
            fclose(file);
            return nullptr;
        }
    }
    return file;
}

size_t FlatFileSeq::Allocate(const CDiskBlockPos& pos, size_t add_size, bool& out_of_space)
{
    out_of_space = false;

    unsigned int n_old_chunks = (pos.nPos + m_chunk_size - 1) / m_chunk_size;
    unsigned int n_new_chunks = (pos.nPos + add_size + m_chunk_size - 1) / m_chunk_size;
    if (n_new_chunks > n_old_chunks) {
        size_t old_size = pos.nPos;
        size_t new_size = n_new_chunks * m_chunk_size;
        size_t inc_size = new_size - old_size;

        if (CheckDiskSpace(m_dir, inc_size)) {
            FILE *file = Open(pos);
            if (file) {
                LogPrintf("Pre-allocating up to position 0x%x in %s%05u.dat\n", new_size, m_prefix, pos.nFile);
                AllocateFileRange(file, pos.nPos, inc_size);
                fclose(file);
                return inc_size;
            }
        } else {
            out_of_space = true;
        }
    }
    return 0;
}

bool FlatFileSeq::Flush(const CDiskBlockPos& pos, bool finalize)
{
    FILE* file = Open(FlatFilePos(pos.nFile, 0)); // Avoid fseek to nPos
    if (!file) {
        return error("%s: failed to open file %d", __func__, pos.nFile);
    }
    if (finalize && !TruncateFile(file, pos.nPos)) {
        fclose(file);
        return error("%s: failed to truncate file %d", __func__, pos.nFile);
    }
    if (!FileCommit(file)) {
        fclose(file);
        return error("%s: failed to commit file %d", __func__, pos.nFile);
    }

    fclose(file);
    return true;
}
