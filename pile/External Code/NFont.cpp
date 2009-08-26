/*
NFont v1.71: A bitmap font class for SDL
by Jonathan Dearborn 2-11-09
Based on SFont (class adapted from Florian Hufsky, jnrdev)
*/
#include "NFont.h"

NFont::NFont()
{
    src = NULL;
    dest = NULL;
    cleanUp = 0;

    maxPos = 0;

    height = 0; // ascent+descent

    maxWidth = 0;
    baseline = 0;
    ascent = 0;
    descent = 0;

    lineSpacing = 0;
    letterSpacing = 0;

    buffer = new char[1024];
}

SDL_Surface* NFont::newColorSurface(Uint32 top, Uint32 bottom, int heightAdjust)
{
    Uint8 tr, tg, tb;

    SDL_GetRGB(top, src->format, &tr, &tg, &tb);

    Uint8 br, bg, bb;

    SDL_GetRGB(bottom, src->format, &br, &bg, &bb);

    SDL_Surface* result = SDL_ConvertSurface(src, src->format, src->flags);

    bool useCK = (result->flags & SDL_SRCALPHA) != SDL_SRCALPHA;  // colorkey if no alpha
    Uint32 colorkey = result->format->colorkey;

    Uint8 r, g, b, a;
    float ratio;
    Uint32 color;
    int temp;

    for (int x = 0, y = 0; y < result->h; x++)
    {
        if (x >= result->w)
        {
            x = 0;
            y++;

            if (y >= result->h)
                break;
        }

        ratio = (y - 2)/float(result->h - heightAdjust);  // the neg 3s are for full color at top and bottom

        if(!useCK)
        {
            color = getPixel(result, x, y);
            SDL_GetRGBA(color, result->format, &r, &g, &b, &a);  // just getting alpha
        }
        else
            a = SDL_ALPHA_OPAQUE;

        // Get and clamp the new values
        temp = int(tr*(1-ratio) + br*ratio);
        r = temp < 0? 0 : temp > 255? 255 : temp;

        temp = int(tg*(1-ratio) + bg*ratio);
        g = temp < 0? 0 : temp > 255? 255 : temp;

        temp = int(tb*(1-ratio) + bb*ratio);
        b = temp < 0? 0 : temp > 255? 255 : temp;


        color = SDL_MapRGBA(result->format, r, g, b, a);


        if(useCK)
        {
            if(getPixel(result, x, y) == colorkey)
                continue;
            if(color == colorkey)
                color == 0? color++ : color--;
        }

        // make sure it isn't pink
        if(color == SDL_MapRGBA(result->format, 0xFF, 0, 0xFF, a))
            color--;
        if(getPixel(result, x, y) == SDL_MapRGBA(result->format, 0xFF, 0, 0xFF, SDL_ALPHA_OPAQUE))
            continue;

        int bpp = result->format->BytesPerPixel;
        Uint8* bits = ((Uint8 *)result->pixels) + y*result->pitch + x*bpp;

        /* Set the pixel */
        switch(bpp) {
            case 1:
                *((Uint8 *)(bits)) = (Uint8)color;
                break;
            case 2:
                *((Uint16 *)(bits)) = (Uint16)color;
                break;
            case 3: { /* Format/endian independent */
                r = (color >> result->format->Rshift) & 0xFF;
                g = (color >> result->format->Gshift) & 0xFF;
                b = (color >> result->format->Bshift) & 0xFF;
                *((bits)+result->format->Rshift/8) = r;
                *((bits)+result->format->Gshift/8) = g;
                *((bits)+result->format->Bshift/8) = b;
                }
                break;
            case 4:
                *((Uint32 *)(bits)) = (Uint32)color;
                break;
        }

    }

    return result;

}

#ifdef NF_USE_TTF
void NFont::loadTTF(TTF_Font* ttf, SDL_Color fg, SDL_Color bg)
{
    if(ttf == NULL)
        return;
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Shaded(ttf, buff, fg, bg);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    #endif
    Uint32 pink = SDL_MapRGB(result->format, 255, 0, 255);
    Uint32 bgcolor = SDL_MapRGB(result->format, bg.r, bg.g, bg.b);
    
    SDL_Rect pixel = {1, 0, 1, 1};
    SDL_Rect line = {1, 0, 1, result->h};
    
    int x = 1;
    SDL_Rect dest = {x, 0, 0, 0};
    for(int i = 0; i < 127 - 33; i++)
    {
        pixel.x = line.x = x-1;
        SDL_FillRect(result, &line, bgcolor);
        SDL_FillRect(result, &pixel, pink);
        
        SDL_BlitSurface(surfs[i], NULL, result, &dest);
        
        x += surfs[i]->w + 1;
        dest.x = x;
        
        SDL_FreeSurface(surfs[i]);
    }
    pixel.x = line.x = x-1;
    SDL_FillRect(result, &line, bgcolor);
    SDL_FillRect(result, &pixel, pink);
    
    setFont(result, true);
}


void NFont::loadTTF(TTF_Font* ttf, SDL_Color fg)
{
    if(ttf == NULL)
        return;
    SDL_Surface* surfs[127 - 33];
    int width = 0;
    int height = 0;
    
    char buff[2];
    buff[1] = '\0';
    for(int i = 0; i < 127 - 33; i++)
    {
        buff[0] = i + 33;
        surfs[i] = TTF_RenderText_Blended(ttf, buff, fg);
        width += surfs[i]->w;
        height = (height < surfs[i]->h)? surfs[i]->h : height;
    }
    
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,width + 127 - 33 + 1,height,32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
    Uint32 pink = SDL_MapRGBA(result->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    SDL_SetAlpha(result, 0, SDL_ALPHA_OPAQUE);
    
    SDL_Rect pixel = {1, 0, 1, 1};
    
    int x = 1;
    SDL_Rect dest = {x, 0, 0, 0};
    for(int i = 0; i < 127 - 33; i++)
    {
        pixel.x = x-1;
        SDL_FillRect(result, &pixel, pink);
        
        SDL_SetAlpha(surfs[i], 0, SDL_ALPHA_OPAQUE);
        SDL_BlitSurface(surfs[i], NULL, result, &dest);
        
        x += surfs[i]->w + 1;
        dest.x = x;
        
        SDL_FreeSurface(surfs[i]);
    }
    pixel.x = x-1;
    SDL_FillRect(result, &pixel, pink);
    
    SDL_SetAlpha(result, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    
    setFont(result, true);
}
#endif

void NFont::drawToSurface(int x, int y, const char* text)
{
    const char* c = text;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || src == NULL || dest == NULL)
        return;
    
    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    dstRect.x = x;
    dstRect.y = y;
    
    int newlineX = x;
    
    for(; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            dstRect.x = newlineX;
            dstRect.y += height + lineSpacing;
            continue;
        }
        
        if (*c == ' ')
        {
            dstRect.x += charWidth[0] + letterSpacing;
            continue;
        }
        unsigned char ctest = (unsigned char)(*c);
        // Skip bad characters
        if(ctest < 33 || (ctest > 126 && ctest < 161))
            continue;
        if(dstRect.x >= dest->w)
            continue;
        if(dstRect.y >= dest->h)
            continue;
        
        num = ctest - 33;  // Get array index
        if(num > 126) // shift the extended characters down to the correct index
            num -= 34;
        srcRect.x = charPos[num];
        srcRect.w = dstRect.w = charWidth[num];
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        dstRect.x += dstRect.w + letterSpacing;
    }
    
}

char* NFont::copyString(const char* c)
{
    if(c == NULL) return NULL;

    int count = 0;
    for(; c[count] != '\0'; count++);

    char* result = new char[count+1];

    for(int i = 0; i < count; i++)
    {
        result[i] = c[i];
    }

    result[count] = '\0';
    return result;
}

void NFont::draw(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    drawToSurface(x, y, buffer);
}

void NFont::drawCenter(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    char* str = copyString(buffer);
    char* del = str;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawToSurface(x - getWidth("%s", str)/2, y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    drawToSurface(x - getWidth("%s", str)/2, y, str);

    delete[] del;
}

void NFont::drawRight(int x, int y, const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    char* str = copyString(buffer);
    char* del = str;

    for(char* c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            drawToSurface(x - getWidth("%s", str), y, str);
            *c = '\n';
            c++;
            str = c;
            y += height;
        }
        else
            c++;
    }
    drawToSurface(x - getWidth("%s", str), y, str);

    delete[] del;
}

int NFont::getHeight(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return height;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    int numLines = 1;
    const char* c;

    for (c = buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
            numLines++;
    }

    //   Actual height of letter region + line spacing
    return height*numLines + lineSpacing*(numLines - 1);  //height*numLines;
}

Uint32 NFont::getPixel(SDL_Surface *Surface, int x, int y)  // No Alpha?
{
    Uint8* bits;
    Uint32 bpp;

    if(x < 0 || x >= Surface->w)
        return 0;  // Best I could do for errors

    bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8*)Surface->pixels) + y*Surface->pitch + x*bpp;

    switch (bpp)
    {
        case 1:
            return *((Uint8*)Surface->pixels + y * Surface->pitch + x);
            break;
        case 2:
            return *((Uint16*)Surface->pixels + y * Surface->pitch/2 + x);
            break;
        case 3:
            // Endian-correct, but slower
            Uint8 r, g, b;
            r = *((bits)+Surface->format->Rshift/8);
            g = *((bits)+Surface->format->Gshift/8);
            b = *((bits)+Surface->format->Bshift/8);
            return SDL_MapRGB(Surface->format, r, g, b);
            break;
        case 4:
            return *((Uint32*)Surface->pixels + y * Surface->pitch/4 + x);
            break;
    }

    return 0;  // Best I could do for errors
}

int NFont::getWidth(const char* formatted_text, ...)
{
    if (formatted_text == NULL)
        return 0;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);

    const char* c;
    int charnum = 0;
    int width = 0;
    int bigWidth = 0;  // Allows for multi-line strings

    for (c = buffer; *c != '\0'; c++)
    {
        charnum = (unsigned char)(*c) - 33;

        // skip spaces and nonprintable characters
        if(*c == '\n')
        {
            bigWidth = bigWidth >= width? bigWidth : width;
            width = 0;
        }
        else if (*c == ' ' || charnum > 222)
        {
            width += charWidth[0];
            continue;
        }

        width += charWidth[charnum];
    }
    bigWidth = bigWidth >= width? bigWidth : width;

    return bigWidth;
}

bool NFont::setFont(SDL_Surface* FontSurface, bool CleanUp)
{
    if(cleanUp)
        SDL_FreeSurface(src);
    src = FontSurface;
    if (src == NULL)
    {
        printf("\n ERROR: NFont given a NULL surface\n");
        cleanUp = 0;
        return 0;
    }

    cleanUp = CleanUp;

    int x = 1, i = 0;
    
    // memset would be faster
    for(int j = 0; j < 256; j++)
    {
        charWidth[j] = 0;
        charPos[j] = 0;
    }

    SDL_LockSurface(src);

    Uint32 pixel = SDL_MapRGB(src->format, 255, 0, 255); // pink pixel
    
    maxWidth = 0;
    
    // Get the character positions and widths
    while (x < src->w)
    {
        if(getPixel(src, x, 0) != pixel)
        {
            charPos[i] = x;
            charWidth[i] = x;
            while(x < src->w && getPixel(src, x, 0) != pixel)
                x++;
            charWidth[i] = x - charWidth[i];
            if(charWidth[i] > maxWidth)
                maxWidth = charWidth[i];
            i++;
        }

        x++;
    }

    maxPos = x - 1;


    pixel = getPixel(src, 0, src->h - 1);
    int j;
    setBaseline();
    
    // Get the max ascent
    j = 1;
    while(j < baseline && j < src->h)
    {
        x = 0;
        while(x < src->w)
        {
            if(getPixel(src, x, j) != pixel)
            {
                ascent = baseline - j;
                j = src->h;
                break;
            }
            x++;
        }
        j++;
    }
    
    // Get the max descent
    j = src->h - 1;
    while(j > 0 && j > baseline)
    {
        x = 0;
        while(x < src->w)
        {
            if(getPixel(src, x, j) != pixel)
            {
                descent = j - baseline+1;
                j = 0;
                break;
            }
            x++;
        }
        j--;
    }
    
    
    height = ascent + descent;
    

    if((src->flags & SDL_SRCALPHA) != SDL_SRCALPHA)
    {
        pixel = getPixel(src, 0, src->h - 1);
        SDL_UnlockSurface(src);
        SDL_SetColorKey(src, SDL_SRCCOLORKEY, pixel);
    }
    else
        SDL_UnlockSurface(src);

    return 1;
}



int NFont::setBaseline(int Baseline)
{
    if(Baseline >= 0)
        baseline = Baseline;
    else
    {
        // Get the baseline by checking a, b, and c and averaging their lowest y-value.
        // Is there a better way?
        Uint32 pixel = getPixel(src, 0, src->h - 1);
        int heightSum = 0;
        int x, i, j;
        for(unsigned char avgChar = 64; avgChar < 67; avgChar++)
        {
            x = charPos[avgChar];
            
            j = src->h - 1;
            while(j > 0)
            {
                i = x;
                while(i - x < charWidth[64])
                {
                    if(getPixel(src, i, j) != pixel)
                    {
                        heightSum += j;
                        j = 0;
                        break;
                    }
                    i++;
                }
                j--;
            }
        }
        baseline = int(heightSum/3.0f + 0.5f);  // Round up and cast
    }
    return baseline;
}

int NFont::getAscent(const char character)
{
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max ascent
    int x = charPos[num];
    int i, j = 1, result = 0;
    Uint32 pixel = getPixel(src, 0, src->h - 1); // bg pixel
    while(j < baseline && j < src->h)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(src, i, j) != pixel)
            {
                result = baseline - j;
                j = src->h;
                break;
            }
            i++;
        }
        j++;
    }
    return result;
}

int NFont::getAscent(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return ascent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = buffer;
    
    for (; *c != '\0'; c++)
    {
        int asc = getAscent(*c);
        if(asc > max)
            max = asc;
    }
    return max;
}

int NFont::getDescent(const char character)
{
    unsigned char test = (unsigned char)character;
    if(test < 33 || test > 222 || (test > 126 && test < 161))
        return 0;
    unsigned char num = (unsigned char)character - 33;
    // Get the max descent
    int x = charPos[num];
    int i, j = src->h - 1, result = 0;
    Uint32 pixel = getPixel(src, 0, src->h - 1); // bg pixel
    while(j > 0 && j > baseline)
    {
        i = charPos[num];
        while(i < x + charWidth[num])
        {
            if(getPixel(src, i, j) != pixel)
            {
                result = j - baseline;
                j = 0;
                break;
            }
            i++;
        }
        j--;
    }
    return result;
}

int NFont::getDescent(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return descent;
    
    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(buffer, formatted_text, lst);
    va_end(lst);
    
    int max = 0;
    const char* c = buffer;
    
    for (; *c != '\0'; c++)
    {
        int des = getDescent(*c);
        if(des > max)
            max = des;
    }
    return max;
}




#ifdef USE_NFONTANIM

NFontAnim::NFontAnim()
{
    data.font = this;
    data.dest = NULL;
    data.maxX = 0;
    data.src = NULL;
    data.height = 0;
    cleanUp = 0;
    data.text = NULL;

    src = NULL;
    dest = NULL;
    maxPos = 0;
    height = 0;
    buffer = new char[1024];
}

void NFontAnim::drawToSurfacePos(int x, int y, NFontAnim_Fn posFn)
{
    data.dest = dest;
    data.src = src;
    data.text = buffer;  // Buffer for efficient drawing
    data.height = height;
    data.charPos = charPos;
    data.charWidth = charWidth;
    data.maxX = maxPos;


    data.index = -1;
    data.letterNum = 0;
    data.wordNum = 1;
    data.lineNum = 1;
    data.startX = x;  // used as reset value for line feed
    data.startY = y;
    
    int preFnX = x;
    int preFnY = y;
    
    const char* c = buffer;
    unsigned char num;
    SDL_Rect srcRect, dstRect, copyS, copyD;
    
    if(c == NULL || src == NULL || dest == NULL)
        return;
    
    srcRect.y = baseline - ascent;
    srcRect.h = dstRect.h = height;
    dstRect.x = x;
    dstRect.y = y;
    
    for(; *c != '\0'; c++)
    {
        data.index++;
        data.letterNum++;
        
        if(*c == '\n')
        {
            data.letterNum = 1;
            data.wordNum = 1;
            data.lineNum++;

            x = data.startX;  // carriage return
            y += height + lineSpacing;
            continue;
        }
        if (*c == ' ')
        {
            data.letterNum = 1;
            data.wordNum++;
            
            x += charWidth[0] + letterSpacing;
            continue;
        }
        unsigned char ctest = (unsigned char)(*c);
        // Skip bad characters
        if(ctest < 33 || (ctest > 126 && ctest < 161))
            continue;
        //if(x >= dest->w) // This shouldn't be used with position control
        //    continue;
        num = ctest - 33;
        if(num > 126) // shift the extended characters down to the array index
            num -= 34;
        srcRect.x = charPos[num];
        srcRect.w = dstRect.w = charWidth[num];
        
        preFnX = x;  // Save real position
        preFnY = y;

        // Use function pointer to get final x, y values
        posFn(x, y, data);
        
        dstRect.x = x;
        dstRect.y = y;
        
        copyS = srcRect;
        copyD = dstRect;
        SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        srcRect = copyS;
        dstRect = copyD;
        
        x = preFnX;  // Restore real position
        y = preFnY;
        
        x += dstRect.w + letterSpacing;
    }
    
}

void NFontAnim::drawPos(int x, int y, NFontAnim_Fn posFn, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = NULL;
    drawToSurfacePos(x, y, posFn);
}

void NFontAnim::drawPosX(int x, int y, NFontAnim_Fn posFn, void* userVar, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = userVar;
    drawToSurfacePos(x, y, posFn);
}

void NFontAnim::drawAll(int x, int y, NFontAnim_Fn allFn, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = NULL;
    allFn(x, y, data);
}

void NFontAnim::drawAllX(int x, int y, NFontAnim_Fn allFn, void* userVar, const char* text, ...)
{
    va_list lst;
    va_start(lst, text);
    vsprintf(buffer, text, lst);
    va_end(lst);

    data.userVar = userVar;
    allFn(x, y, data);
}

#endif


