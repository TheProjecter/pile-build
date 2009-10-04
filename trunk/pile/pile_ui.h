#ifndef _PILE_UI_H__
#define _PILE_UI_H__

#include <string>

#define PILE_PRINT_BUFFER_SIZE 4096

//#define PILE_NO_GUI

#define PILE_DEBUG_PILE
#define PILE_DEBUG_TOKENS

extern char ui_buffer[PILE_PRINT_BUFFER_SIZE];

extern bool ui_print;
extern bool ui_warning;
extern bool ui_error;
extern bool ui_debug;
extern bool ui_log;

extern bool ui_log_print;
extern bool ui_log_warning;
extern bool ui_log_error;
extern bool ui_log_debug;


class Configuration;

bool UI_init(bool graphical, Configuration& config);

void UI_quit();

void UI_print(const char* formatted_text, ...);

void UI_output(const char* formatted_text, ...);
void UI_warning(const char* formatted_text, ...);
void UI_error(const char* formatted_text, ...);
void UI_debug(const char* formatted_text, ...);
void UI_debug_pile(const char* formatted_text, ...);

void UI_log(const char* formatted_text, ...);

void UI_print_file(std::string filename);

int UI_choice(int numChoices, std::string* choices);

std::string UI_input();

bool UI_prompt(std::string message);
std::string UI_promptString(std::string message);

void UI_updateScreen();
int UI_waitKeyPress();
int UI_processEvents();



#ifdef PILE_DEBUG_PILE

extern bool ui_debug_pile;
extern bool ui_log_debug_pile;

#endif

#endif
