#include "gba.h"

volatile unsigned short *videoBuffer = (volatile unsigned short *) 0x6000000;
u32 vBlankCounter = 0;

void waitForVBlank(void) {
    while (SCANLINECOUNTER > 160);
    while (SCANLINECOUNTER < 160);
    vBlankCounter++;
}

static int __qran_seed = 42;
static int qran(void) {
    __qran_seed = 1664525 * __qran_seed + 1013904223;
    return (__qran_seed >> 16) & 0x7FFF;
}

void setseed(u32 seed) {
    __qran_seed = (int) seed;
}

int randint(int min, int max) { return (qran() * (max - min) >> 15) + min; }

void setPixel(int row, int col, u16 color) {
    *(videoBuffer + OFFSET(row, col, WIDTH) ) = color;
}

void drawRectDMA(int row, int col, int width, int height, volatile u16 color) {
    for(int r=0; r<height; r++) {

//        Naive Solution (non DMA)
//        for(int c=0; c<width; c++)
//            setPixel(row+r, col+c, color);

        DMA[3].src = &color;
        DMA[3].dst = &videoBuffer[OFFSET(row+r,col,WIDTH)];
        DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT;
    }
}

void drawFullScreenImageDMA(const u16 *image) {
    DMA[3].src = image;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = (WIDTH*HEIGHT) | DMA_ON | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT;
}

void drawImageDMA(int row, int col, int width, int height, const u16 *image) {
    for(int r=0; r<height; r++) {
        DMA[3].src = &image[r*width];
        DMA[3].dst = &videoBuffer[OFFSET(row+r,col,WIDTH)];
        DMA[3].cnt = width | DMA_ON | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT;
    }
}

// TODO what
void undrawImageDMA(int row, int col, int width, int height, const u16 *image) {
    for(int r=0; r<height; r++) {
        DMA[3].src = &image[r*width];
        DMA[3].dst = &videoBuffer[OFFSET(row+r,col,WIDTH)];
        DMA[3].cnt = width | DMA_ON | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT;
    }
}

void fillScreenDMA(volatile u16 color) {
    DMA[3].src = &color;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = (WIDTH*HEIGHT) | DMA_ON | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT;
}

void drawChar(int row, int col, char ch, u16 color) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            if (fontdata_6x8[OFFSET(j, i, 6) + ch * 48]) {
                setPixel(row + j, col + i, color);
            }
        }
    }
}

void drawString(int row, int col, char *str, u16 color) {
    while (*str) {
        drawChar(row, col, *str++, color);
        col += 6;
    }
}

void drawCenteredString(int row, int col, int width, int height, char *str, u16 color) {
    u32 len = 0;
        char *strCpy = str;
        while (*strCpy) {
            len++;
            strCpy++;
        }

    u32 strWidth = 6 * len;
    u32 strHeight = 8;

    int new_row = row + ((height - strHeight) >> 1);
    int new_col = col + ((width - strWidth) >> 1);
    drawString(new_row, new_col, str, color);
}



void drawImageFadeInOut(u32 delay, u16 loadDuration, u16 startColor, const u16 *image) {
    UNUSED(delay);
    fillScreenDMA(startColor);
    u16 buffer[38400];
    for (int i = 0; i < 38400; i++) {
        buffer[i] = startColor;
    }
    u8 progress = 0;
    while (progress < loadDuration) {
        for (int i = 0; i < 38400; i++) {
            u32 r = ((buffer[i] & 0x1F) << 8) + ((image[i] & 0x1F) << 8) * progress / loadDuration;
            u32 g = (((buffer[i] >> 5) & 0x1F) << 8) + (((image[i] >> 5) & 0x1F) << 8) * progress / loadDuration;
            u32 b = (((buffer[i] >> 10) & 0x1F) << 8) + (((image[i] >> 10) & 0x1F) << 8) * progress / loadDuration;
            buffer[i] = COLOR(r >> 8, g >> 8, b >> 8);
        }
        drawFullScreenImageDMA(buffer);
        progress++;
        waitForVBlank();
    }
    
}
