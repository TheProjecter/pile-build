/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

main.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains main(), which directs the command-line argument handling,
config loading, GUI loading, and interpreter calls.
*/

#include "pile_global.h"
#include "pile_env.h"
#include "pile_config.h"
#include "pile_depend.h"
#include "pile_commands.h"
#include "pile_build.h"
#include "pile_load.h"
#include "pile_ui.h"
#include "string_functions.h"

Environment env;
Configuration config;

void printDepends(const string& file);
bool interpret(string filename, Environment& env, Configuration& config);


extern string log_file;

bool isSourceFile(const string& file);


list<string> getLocalSourceFiles()
{
    list<string> result;
    list<string> ls = ioList(".");
    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        if(isSourceFile(*e))
            result.push_back(*e);
    }
    return result;
}



/*
Finds a pilefile, starting with "com.pile", then the first *.pile that it finds.

Takes: -
Returns: string (file name)
*/
string findPileFile()
{
    if(ioExists("com.pile"))
        return "com.pile";
    string file;
        
    list<string> ls = ioList(".", false, true);

    for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        unsigned int dotpos = e->find_last_of(".");
        if(dotpos != string::npos && e->substr(dotpos, string::npos) == ".pile")
        {
            file = *e;
            break;
        }
    }
    return file;
}



// Types: 0=Empty, 1=Compile
void generatePilefile(string name, int type)
{
    string base = ioStripToFile(name);
    if(ioStripToExt(name) != "pile")
        name += ".pile";
    else
    {
        if(name == "com.pile")
            base = "a.out";
        else
        {
            // Strip extension
            unsigned int dot = base.find_last_of('.');
            if(dot != string::npos)
                base = base.substr(0, dot);
        }
    }
        
    if(ioExists(name))
    {
        UI_print("Cannot create new pilefile.  %s already exists!\n", name.c_str());
        return;
    }
    
    ioNew(name);
    
    switch(type)
    {
        case 0:
            break;
        case 1:
        default:
            ioAppend("OUTPUT = \"" + base + "\"\n\nSOURCES += ls(\"*.cpp\")\n\nLFLAGS += [\"-lSDLmain\", \"-lSDL\"]\n\nCFLAGS += [\"`sdl-config --cflags`\"]\n\ncpp_compiler.scan(SOURCES)\n\nOBJECTS += cpp_compiler.compile(SOURCES, CFLAGS)\n\ncpp_linker.link(OUTPUT, OBJECTS, LIBRARIES, LFLAGS)", name);
        break;
    }
}


int main(int argc, char* argv[])
{
    
    //string pileDirectory = ioGetProgramPath();
    
    // Create config dir
    ioNewDir(getConfigDir());
    
    // Set log file
    //log_file = getConfigDir() + "/pile_log.txt";
    log_file = "pile_log.txt";
    
    // Delete log file
    ioDelete(log_file);
    
    UI_debug_pile("Starting up...\n");
    UI_debug_pile("Argc = %d\n", argc);
    for(int i = 0; i < argc; i++)
    {
        UI_debug_pile("Argv[%d] = %s\n", i, argv[i]);
    }
    
    
    
    
    // Load pile.conf
    string configDir = getHomeDir() + "/.pile/";
    if(!ioExists(configDir))
        ioNewDir(configDir);
    
    loadConfig(configDir, config);
    
    env.loadConfig(config);
    
    string file;  // pilefile name
    
    
    int cleaning = 0;  // Interpret without actions or messages
    bool graphical = false;
    bool promptForNoPilefile = true;
    // Check for graphical flag
    for(int i = 1; i < argc; i++)
    {
        if(string("-g") == argv[i])
        {
            graphical = true;
            if(!UI_init(true, config))
            {
                UI_error("pile GUI failed to initialize.  Exiting...\n");
                return 0;
            }
            UI_print("Pile GUI started.\n\n");
        }
        else if(string("-n") == argv[i])
        {
            promptForNoPilefile = false;
        }
        else if(string("new") == argv[i])
        {
            // Generate a new pilefile
            // FIXME: Allow 'pile new c++', not just 'pile new my.pile c++'
            string name = "com.pile";
            i++;
            if(i < argc)
            {
                name = argv[i];
            }
            int type = 0;
            i++;
            if(i < argc)
            {
                string s = argv[i];
                if(s == "compile" || s == "build" || s == "cpp" || s == "c++")
                    type = 1;
            }
            
            generatePilefile(name, type);
            return 0;
        }
        else if(string("edit") == argv[i])
        {
            string file = "com.pile";
            i++;
            if(i < argc)
            {
                file = argv[i];
                if(file == "pile.conf")
                {
                    file = configDir + "pile.conf";
                }
            }
            edit(file, config);
            return 0;
        }
        else if(string("--version") == argv[i])
        {
            UI_print("pile Version %s\n", getVersion().c_str());
            return 0; // FIXME: Shouldn't always return here.
        }
        else if(string("scan") == argv[i])
        {
            // FIXME: Create/update dependency files (in depends directory?)
            return 0; // FIXME: Shouldn't always return here.
        }
        else if(string("clean") == argv[i])
        {
            if(i+1 < argc && string("all") == argv[i+1])
            {
                cleaning = 1;
                i++;
            }
            else if(i+1 < argc && string("old") == argv[i+1])
            {
                cleaning = 3;
                i++;
            }
            else
            {
                cleaning = 2;
            }
        }
        else if(string("dryrun") == argv[i])
        {
            env.dryRun = true;
        }
        else if(string("--nocompile") == argv[i])
        {
            env.noCompile = true;
        }
        else if(string("--nolink") == argv[i])
        {
            env.noLink = true;
        }
        // Check for .pile file extension ('pile myfile.pile')
        else if(string("pile") == ioStripToExt(argv[i]))
        {
            // Found a pilefile...
            // Change directory first
            ioSetCWD(ioStripToDir(argv[i]));
            file = ioStripToFile(argv[i]);
        }
        else if(string("-v") == argv[i])
        {
            i++;
            if(i >= argc)
                break;
            // Now we grab all the variants from the list here...
            // It could be var1,var2,... or "var1, var2, ..."
            
            list<string> ls = ioExplode(argv[i], ',');
            for(list<string>::iterator e = ls.begin(); e != ls.end(); e++)
            {
                if(*e != "")
                    env.variants.push_back(*e);
            }
        }
        else
        {
            //p
            string arg = argv[i];
            // FIXME: Replace this find() with a smarter one that takes quotes into account
            unsigned int eq = arg.find("=");
            if(eq != string::npos && eq > 0)
            {
                env.variables.insert(make_pair(arg.substr(0, eq), arg.substr(eq+1)));
            }
            else
            {
                UI_warning("pile Warning: Command \"%s\" not found, so it will be ignored.\n", argv[i]);
            }
        }
    }
    
    UI_processEvents();
    UI_updateScreen();
    
    
    
    if(!graphical)
    {
        UI_init(false, config);
    }
    
    UI_debug_pile("Current directory: %s", ioGetCWD().c_str());
    
    
    // Find the appropriate pilefile
    if(file == "")
    {
            UI_debug_pile("No pilefile specified.  Searching...\n");
            file = findPileFile();
    }
    
    bool errorFlag = false;
    bool interpreterError = false;
    // If we've found a Pilefile, then we can begin the build.
    if(file != "")
    {
        UI_debug_pile("Found pilefile: %s\n", file.c_str());
        UI_processEvents();
        UI_updateScreen();
        
        // Interpret the file
        if(!interpret(file, env, config))
            errorFlag = interpreterError = true;
        UI_debug_pile("Done interpreting.\n");
        
        UI_processEvents();
        UI_updateScreen();
        
        if(!errorFlag && env.sources.size() > 0)
        {
            // Scan for dependencies
            if(config.useAutoDepend)
            {
                UI_debug_pile("Scanning.\n");
                for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
                {
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
                    //if(!recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, ""))
                    //{
                    //    errorFlag = true;
                    //    break;
                    //}
                }
            }
            
            if(!env.dryRun && !cleaning && !errorFlag)
            {
                UI_debug_pile("Building and linking.\n");
                if(!env.noCompile && !build(env, config))
                    errorFlag = true;
                if(!errorFlag)
                    if(!env.noLink && !link(config.languages.find("CPP_LINKER_D")->second, env, config))
                        errorFlag = true;
            }
        }
        
    }
    else  // No Pilefile found
    {
        // Prompt user in order to proceed to build all local source files.
        if(!promptForNoPilefile || UI_prompt(" No Pilefile found here.  Should I try to build all local source files?\n"))
        {
            env.sources = getLocalSourceFiles();
            if(env.variables.find("CFLAGS") == env.variables.end())
                config.cflags += UI_promptString(" Compiler flags?\n");
            
            if(env.variables.find("LFLAGS") == env.variables.end())
                config.lflags += UI_promptString(" Linker flags?\n");
            
            // Scan for dependencies
            if(config.useAutoDepend)
            {
                for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
                {
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
                }
            }
            
            UI_debug_pile("Building and linking.\n");
            if(!env.dryRun && !cleaning && !errorFlag)
            {
                if(!env.noCompile && !build(env, config))
                    errorFlag = true;
                if(!errorFlag && !env.noLink && !link(config.languages.find("CPP_LINKER_D")->second, env, config))
                    errorFlag = true;
            }
        }
    }
    
    UI_processEvents();
    UI_updateScreen();
    
    // Cleaning
    // FIXME: This should be the default clean, but there should also be a way to overload it in the Pilefile.
    if(cleaning > 0)
    {
        if(cleaning == 1)
            clean(true, env.sources, config, env.outfile);
        else if(cleaning == 2)
            clean(false, env.sources, config, env.outfile);
        else if(cleaning == 3)
            cleanOld(false, env.sources, config, env.outfile);
        UI_processEvents();
        UI_updateScreen();
    }
    
    
    
    if(errorFlag)
    {
        if(interpreterError)
            UI_error("\nPilefile errors have occurred...\n");
        else
            UI_error("\nBuild errors have occurred...\n");
    }
    
    if(graphical)
    {
        ui_print = true;
        ui_log_print = false;
        if(errorFlag)
            UI_print("\nPress any key to quit.\n");
        else
            UI_print("\nAll done!  Press any key.\n");
        UI_updateScreen();
        
        UI_waitKeyPress();
    }
    
    UI_quit();
    
    return 0;
}
