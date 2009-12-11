/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_ui.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the functions which implement the user interface.
The UI functions can be enabled/disabled and display according to the UI chosen
(graphical or console).
*/

#include "pile_ui.h"
#include "stdarg.h"
#include <iostream>
#include <fstream>
#include "string_functions.h"
#include "pile_config.h"
#include "External Code/goodio.h"

#define PILE_BUTTON_WHEELLEFT 6
#define PILE_BUTTON_WHEELRIGHT 7

char ui_buffer[PILE_PRINT_BUFFER_SIZE];

using namespace std;

string log_file;


bool ui_print = true;
bool ui_warning = true;
bool ui_error = true;
bool ui_debug = false;
bool ui_log = true;


bool ui_log_print = true;
bool ui_log_warning = true;
bool ui_log_error = true;
bool ui_log_debug = true;

#ifndef PILE_NO_GUI


#include "SDL.h"
#include "External Code/NFont.h"


SDL_Surface* screen = NULL;

NFont* blackfont = NULL;

NFont* printfont = NULL;
NFont* outputfont = NULL;
NFont* warningfont = NULL;
NFont* errorfont = NULL;
NFont* debugfont = NULL;

bool ui_gui = false;
float scrollX = 0;
float scrollY = 0;

float dt = 0;
Uint32 lasttime = 0;

class Message
{
    public:
    NFont* font;
    string text;

    Message(const string& text, NFont* font)
        : font(font)
        , text(text)
    {}
};

list<Message> message;

SDL_Color print_color = {0, 0, 0, SDL_ALPHA_OPAQUE};
SDL_Color output_color = {64, 64, 128, SDL_ALPHA_OPAQUE};
SDL_Color warning_color = {200, 128, 0, SDL_ALPHA_OPAQUE};
SDL_Color error_color = {200, 0, 0, SDL_ALPHA_OPAQUE};
SDL_Color debug_color = {0, 0, 200, SDL_ALPHA_OPAQUE};


void pixel(int x, int y, Uint32 color)
{
    SDL_Rect r = {x, y, 1, 1};
    SDL_FillRect(screen, &r, color);
}
void lineh(int x, int y, int x2, Uint32 color)
{
    if(x > x2)
    {
        int s = x;
        x = x2;
        x2 = s;
    }
    SDL_Rect r = {x, y, x2 - x + 1, 1};
    SDL_FillRect(screen, &r, color);
}
void linev(int x, int y, int y2, Uint32 color)
{
    if(y > y2)
    {
        int s = y;
        y = y2;
        y2 = s;
    }
    SDL_Rect r = {x, y, 1, y2 - y + 1};
    SDL_FillRect(screen, &r, color);
}
void rect(int x, int y, int x2, int y2, Uint32 color)
{
    if(x > x2)
    {
        int s = x;
        x = x2;
        x2 = s;
    }
    if(y > y2)
    {
        int s = y;
        y = y2;
        y2 = s;
    }
    SDL_Rect r = {x, y, x2 - x + 1, y2 - y + 1};
    SDL_FillRect(screen, &r, color);
}

string UI_wrap(string text, NFont* font, unsigned int width)
{
    if(font == NULL)
        return text;
    unsigned int iw = 0;
    unsigned int w = 0;

    unsigned int lastLine = 0;

    for(unsigned int i = 0; i < text.size(); i++)
    {
        if(text[i] == '\n')
        {
            lastLine = i+1;
            w = 0;
            continue;
        }

        iw = font->getWidth("%c", text[i]);
        w += iw;
        if(w > width)
        {
            text.insert(i, "\n");
            lastLine = i;
            w = iw;
        }
    }

    return text;
}

#endif


#ifdef PILE_DEBUG_PILE

bool ui_debug_pile = false;
bool ui_log_debug_pile = true;

#endif


bool UI_init(bool graphical, Configuration& config)
{
    #ifndef PILE_NO_GUI
    ui_gui = graphical;
    if(ui_gui)
    {
        int w = 500;
        int h = 300;

        SDL_putenv(const_cast<char*>("SDL_VIDEO_CENTERED=center"));
        if(SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            UI_log("Error: Couldn't initialize SDL: %s\n", SDL_GetError());
            return false;
        }

        screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE);

        if ( screen == NULL ) {
            UI_log("Error: Couldn't set video mode %dx%d: %s\n", w, h, SDL_GetError());
            return false;
        }


        SDL_WM_SetCaption("Pile GUI", NULL);
        //SDL_WM_SetIcon(images.load("pics/icon.png"), NULL);


        if(TTF_Init() == -1)
        {
            UI_log("Error: Unable to initialize SDL_ttf: %s \n", TTF_GetError());
            return false;
        }

        SDL_Color black = {0, 0, 0, 255};
        //SDL_Color blue = {0, 0, 200, 255};
        //SDL_Color gray = {127, 127, 127, 255};
        SDL_Color white = {255, 255, 255, 255};

        int size = 20;

        UI_log("Install path = %s\n", config.installPath.c_str());
        UI_log("Install path2 = %s\n", addDirSlash(config.installPath).c_str());

        TTF_Font* ttf = TTF_OpenFont((addDirSlash(config.installPath) + "fonts/FreeSans.ttf").c_str(), size);

        if(ttf == NULL)
        {
            UI_log("Unable to load font: %s \n", TTF_GetError());
            return false;
        }

        blackfont = new NFont;
        blackfont->setDest(screen);
        blackfont->loadTTF(ttf, black, white);

        printfont = new NFont;
        printfont->setDest(screen);
        printfont->loadTTF(ttf, print_color, white);

        outputfont = new NFont;
        outputfont->setDest(screen);
        outputfont->loadTTF(ttf, output_color, white);

        warningfont = new NFont;
        warningfont->setDest(screen);
        warningfont->loadTTF(ttf, warning_color, white);

        errorfont = new NFont;
        errorfont->setDest(screen);
        errorfont->loadTTF(ttf, error_color, white);

        debugfont = new NFont;
        debugfont->setDest(screen);
        debugfont->loadTTF(ttf, debug_color, white);

        TTF_CloseFont(ttf);

        return true;
    }

    lasttime = SDL_GetTicks();
    #endif

    return true;
}

void UI_quit()
{
    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        delete blackfont;
        delete printfont;
        delete outputfont;
        delete warningfont;
        delete errorfont;
        delete debugfont;
        SDL_Quit();
        return;
    }
    #endif
}


void UI_print(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        if(ui_print)
            message.push_front(Message(ui_buffer, printfont));
        if(ui_log_print)
            UI_log(ui_buffer);
        return;
    }
    #endif
    if(ui_print)
        printf(ui_buffer);

    if(ui_log_print)
        UI_log(ui_buffer);
}

void UI_output(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        if(ui_print || ui_error)
            message.push_front(Message(ui_buffer, outputfont));
        if(ui_log_print || ui_log_error)
            UI_log(ui_buffer);
        return;
    }
    #endif
    if(ui_print || ui_error)
        printf(ui_buffer);

    if(ui_log_print || ui_log_error)
        UI_log(ui_buffer);
}

void UI_warning(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        if(ui_warning)
            message.push_front(Message(ui_buffer, warningfont));
        if(ui_log_warning)
            UI_log(ui_buffer);
        return;
    }
    #endif

    if(ui_warning)
        printf(ui_buffer);

    if(ui_log_warning)
        UI_log(ui_buffer);
}

void UI_error(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        if(ui_error)
            message.push_front(Message(ui_buffer, errorfont));
        if(ui_log_error)
            UI_log(ui_buffer);
        return;
    }
    #endif

    if(ui_error)
        printf(ui_buffer);

    if(ui_log_error)
        UI_log(ui_buffer);
}

void UI_debug(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        if(ui_debug)
            message.push_front(Message(ui_buffer, debugfont));
        if(ui_log_debug)
            UI_log(ui_buffer);
        return;
    }
    #endif

    if(ui_debug)
        printf(ui_buffer);

    if(ui_log_debug)
        UI_log(ui_buffer);
}

void UI_debug_pile(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    #ifndef PILE_NO_GUI
    if(ui_gui)
    {
        #ifdef PILE_DEBUG_PILE
        if(ui_debug_pile)
            message.push_front(Message(ui_buffer, debugfont));
        if(ui_log_debug_pile)
            UI_log(ui_buffer);
        #endif
        return;
    }
    #endif

    #ifdef PILE_DEBUG_PILE
    if(ui_debug_pile)
        printf(ui_buffer);

    if(ui_log_debug_pile)
        UI_log(ui_buffer);
    #endif
}

void UI_log(const char* formatted_text, ...)
{
    if(formatted_text == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsprintf(ui_buffer, formatted_text, lst);
    va_end(lst);

    if(ui_log)
        ioAppend(ui_buffer, log_file.c_str());
}

int UI_choice(int numChoices, string* choices)
{
    #ifndef PILE_NO_GUI
    if(ui_gui)
    {

        return 0;
    }
    #endif

    return 0;
}

string UI_input()
{
    #ifndef PILE_NO_GUI
    if(ui_gui)
    {

        return "";
    }
    #endif

    return "";
}

bool isYes(string text)
{
    toLower(text);
    return (text == "y" || text == "yes");
}
bool isNo(string text)
{
    toLower(text);
    return (text == "n" || text == "no");
}

bool UI_prompt(string message)
{
    #ifndef PILE_NO_GUI
    if(ui_gui)
    {

        return false;
    }
    #endif
    printf("%s (Yes/No): ", message.c_str());
    string user;
    bool ran = false;
    do
    {
        if(ran)
            printf(" Huh?: ");
        ran = true;
        //cin.clear();  // :( This didn't work...
        cin >> user;
        if(user[user.size()-1] == '\n')
            user.erase(user.end()-1);
    }
    while(!(isYes(user) || isNo(user)));
    return isYes(user);
}

string UI_promptString(string message)
{
    #ifndef PILE_NO_GUI
    if(ui_gui)
    {

        return "";
    }
    #endif
    printf("%s: ", message.c_str());
    char buff[1024];
    cin.clear();
    do
    {
        cin.getline(buff, 1023);
        delay(20);
    }
    while(strcmp(buff, "") == 0);

    return buff;
}


void UI_updateScreen()
{
    #ifndef PILE_NO_GUI
    if(screen == NULL)
        return;
    SDL_FillRect(screen, NULL, 0xffffff);

    int maxX = 0;
    int width = 0;

    int x = 20;
    int y = screen->h - 11;  // Give room for the scrollbar
    // FIXME: Count the number of newlines in the string and adjust y according to that.
    // FIXME: Use the width of the last line in the string for the next one. (don't ignore the last newline)
    // FIXME: Needs word wrap.
    for(list<Message>::iterator e = message.begin(); e != message.end(); e++)
    {
        NFont* font = e->font;
        if(font == NULL)
            continue;
        //if(!continuing)
            x = 20;
        y -= font->getHeight(e->text.c_str());

        // Trailing newlines...
        if(e->text.size() > 0 && e->text[e->text.size()-1] == '\n')
        {
            x = 20;
            y += font->getHeight();
        }

        font->draw(int(20 - scrollX), int(y - scrollY), e->text.c_str());
        if(y < scrollY)
            break;

        width = font->getWidth(e->text.c_str());
        x += width;

        if(width > maxX)
            maxX = width;
    }

    // Draw the horizontal scrollbar
    // FIXME: Word wrap should be toggleable
    if(maxX > screen->w)
    {
        rect(0, screen->h - 11, screen->w, screen->h, 0xffffff);
        lineh(0, screen->h - 11, screen->w, 0x000000);
        float pos = (float(scrollX)/maxX)*(screen->w-1);
        rect(int(pos - 5), screen->h - 11, int(pos + 5), screen->h, 0x000000);
    }

    SDL_Flip(screen);
    #endif
}

int UI_waitKeyPress()
{
    #ifndef PILE_NO_GUI
    Uint8* keystates = SDL_GetKeyState(NULL);

    float vel = 40;

    SDL_Event event;
    while(1)
    {
        Uint32 starttime = SDL_GetTicks();
        dt = (starttime - lasttime)/1000.0f;
        lasttime = starttime;

        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                return SDLK_UNKNOWN;
            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_UP:
                    case SDLK_DOWN:
                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        break;
                    default:
                        return event.key.keysym.sym;
                }
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.button == SDL_BUTTON_WHEELUP)
                {
                    scrollY -= 20;
                }
                else if(event.button.button == SDL_BUTTON_WHEELDOWN)
                {
                    scrollY += 20;
                }
                else if(event.button.button == PILE_BUTTON_WHEELLEFT)
                {
                    scrollX -= 20;
                }
                else if(event.button.button == PILE_BUTTON_WHEELRIGHT)
                {
                    scrollX += 20;
                }
            }
        }
        if(keystates[SDLK_UP])
            scrollY -= vel*dt;
        if(keystates[SDLK_DOWN])
            scrollY += vel*dt;
        if(keystates[SDLK_LEFT])
            scrollX -= vel*dt;
        if(keystates[SDLK_RIGHT])
            scrollX += vel*dt;
        if(keystates[SDLK_UP] || keystates[SDLK_DOWN] || keystates[SDLK_LEFT] || keystates[SDLK_RIGHT])
        {
            vel += 200*dt;
        }
        else
            vel = 200*dt;
        if(scrollX < 0)
            scrollX = 0;

        UI_updateScreen();
        SDL_Delay(50);
    }
    #else
    //getch();
    return 0;
    #endif
}


int UI_processEvents()
{
    #ifndef PILE_NO_GUI
    Uint8* keystates = SDL_GetKeyState(NULL);

    static float vel = 40;

    Uint32 starttime = SDL_GetTicks();
    dt = (starttime - lasttime)/1000.0f;
    lasttime = starttime;

    SDL_Event event;

    bool continuousInput = false;

    do
    {
        continuousInput = false;

        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                return -1;
            if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    return -1;
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.button == SDL_BUTTON_WHEELUP)
                {
                    scrollY -= 20;
                }
                else if(event.button.button == SDL_BUTTON_WHEELDOWN)
                {
                    scrollY += 20;
                }
                else if(event.button.button == PILE_BUTTON_WHEELLEFT)
                {
                    scrollX -= 20;
                }
                else if(event.button.button == PILE_BUTTON_WHEELRIGHT)
                {
                    scrollX += 20;
                }
            }
        }

        if(keystates[SDLK_UP])
            scrollY -= vel*dt;
        if(keystates[SDLK_DOWN])
            scrollY += vel*dt;
        if(keystates[SDLK_LEFT])
            scrollX -= vel*dt;
        if(keystates[SDLK_RIGHT])
            scrollX += vel*dt;
        if(keystates[SDLK_UP] || keystates[SDLK_DOWN] || keystates[SDLK_LEFT] || keystates[SDLK_RIGHT])
        {
            vel += 5*dt;
            continuousInput = true;
        }
        else
            vel = 40;
        if(scrollX < 0)
            scrollX = 0;

        UI_updateScreen();
        SDL_Delay(50);
    }
    while(continuousInput);
    #endif

    return 0;
}



void UI_print_file(string filename)
{
    UI_debug_pile("Printing file: %s\n", filename.c_str());
    ifstream fin;
    fin.open(filename.c_str());
    if(fin.fail())
    {
        UI_debug_pile(" Failed to open file\n", filename.c_str());
        fin.close();
        return;
    }

    bool printed = false;
    string line;

    while(!fin.eof())
    {
        getline(fin, line);
        UI_output((line + "\n").c_str());
        printed = true;
    }

    fin.close();
}


