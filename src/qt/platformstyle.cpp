// Copyright (c) 2015-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/platformstyle.h>

#include <QApplication>
#include <QColor>
#include <QImage>
#include <QPalette>

static const struct {
    const char *platformId;
    /** Show images on push buttons */
    const bool imagesOnButtons;
    /** Colorize single-color icons */
    const bool colorizeIcons;
    /** Extra padding/spacing in transactionview */
    const bool useExtraSpacing;
} platform_styles[] = {
    {"macosx", false, false, true},
    {"windows", true, false, false},
    /* Other: linux, unix, ... */
    {"other", true, true, false}
};

namespace {
/* Local functions for colorizing single-color images */

void MakeSingleColorImage(QImage& img, const QColor& colorbase)
{
    img = img.convertToFormat(QImage::Format_ARGB32);
    for (int x = img.width(); x--; )
    {
        for (int y = img.height(); y--; )
        {
            const QRgb rgb = img.pixel(x, y);
            img.setPixel(x, y, qRgba(colorbase.red(), colorbase.green(), colorbase.blue(), qAlpha(rgb)));
        }
    }
}

QIcon ColorizeIcon(const QIcon& ico, const QColor& colorbase)
{
    QIcon new_ico;
    for (const QSize& sz : ico.availableSizes())
    {
        QImage img(ico.pixmap(sz).toImage());
        MakeSingleColorImage(img, colorbase);
        new_ico.addPixmap(QPixmap::fromImage(img));
    }
    return new_ico;
}

QImage ColorizeImage(const QString& filename, const QColor& colorbase)
{
    QImage img(filename);
    MakeSingleColorImage(img, colorbase);
    return img;
}

QIcon ColorizeIcon(const QString& filename, const QColor& colorbase)
{
    return QIcon(QPixmap::fromImage(ColorizeImage(filename, colorbase)));
}

}


PlatformStyle::PlatformStyle(const QString &_name, bool _imagesOnButtons, bool _colorizeIcons, bool _useExtraSpacing):
    name(_name),
    imagesOnButtons(_imagesOnButtons),
    colorizeIcons(_colorizeIcons),
    useExtraSpacing(_useExtraSpacing),
    singleColor(0,0,0),
    textColor(0,0,0)
{
    // Determine icon highlighting color
    if (colorizeIcons) {
        const QColor colorHighlightBg(QApplication::palette().color(QPalette::Highlight));
        const QColor colorHighlightFg(QApplication::palette().color(QPalette::HighlightedText));
        const QColor colorText(QApplication::palette().color(QPalette::WindowText));
        const int colorTextLightness = colorText.lightness();
        QColor colorbase;
        if (abs(colorHighlightBg.lightness() - colorTextLightness) < abs(colorHighlightFg.lightness() - colorTextLightness))
            colorbase = colorHighlightBg;
        else
            colorbase = colorHighlightFg;
        singleColor = colorbase;
    }
    // Determine text color
    textColor = QColor(QApplication::palette().color(QPalette::WindowText));
}

QImage PlatformStyle::SingleColorImage(const QString& filename) const
{
    if (!colorizeIcons)
        return QImage(filename);
    return ColorizeImage(filename, SingleColor());
}

QIcon PlatformStyle::SingleColorIcon(const QString& filename) const
{
    if (!colorizeIcons)
        return QIcon(filename);
    return ColorizeIcon(filename, SingleColor());
}

QIcon PlatformStyle::SingleColorIcon(const QIcon& icon) const
{
    if (!colorizeIcons)
        return icon;
    return ColorizeIcon(icon, SingleColor());
}

QIcon PlatformStyle::TextColorIcon(const QIcon& icon) const
{
    return ColorizeIcon(icon, TextColor());
}

const PlatformStyle *PlatformStyle::instantiate(const QString &platformId)
{
    for (const auto& platform_style : platform_styles) {
        if (platformId == platform_style.platformId) {
            return new PlatformStyle(
                    platform_style.platformId,
                    platform_style.imagesOnButtons,
                    platform_style.colorizeIcons,
                    platform_style.useExtraSpacing);
        }
    }
    return nullptr;
}

