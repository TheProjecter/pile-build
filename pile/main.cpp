#include "pile_global.h"
#include "pile_env.h"
#include "pile_config.h"
#include "pile_depend.h"
#include "pile_commands.h"
#include "pile_build.h"
#include "pile_load.h"
#include "pile_ui.h"
#include "string_functions.h"

void printDepends(const string& file);
bool interpret(string filename, Environment& env);


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

/*
Creates a name for an executable by changing the file extension.

Takes: string (file names to be converted)
Returns: string (converted name)
*/
string getExeName(string file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
    {
        return (file.substr(0, dotpos) + EXE_EXT);
    }
    else
    {
        return (file + EXE_EXT);
    }
}



void removePath(string& file)
{
    unsigned int lastSlash = file.find_last_of('/');
    if(lastSlash != string::npos)
    {
        file = file.substr(lastSlash+1, string::npos);
    }
}

/*
Combines two lists to make a full list of object file names.

Takes: list<string> (source file names to convert into object file names)
       list<string> (object file names)
Returns: string (All object file names separated by spaces)
*/
string getObjectString(const list<string>& sources, const list<string>& objects)
{
    string objectstr;
    
    for(list<string>::const_iterator e = sources.begin(); e != sources.end(); e++)
    {
        string obj = *e;
        unsigned int dotpos = e->find_last_of(".");
        if(dotpos != string::npos)  // Perhaps unneccessary
        {
            obj = obj.substr(0, dotpos) + ".o";
            removePath(obj);
            objectstr += (obj + " ");
        }
    }
    
    for(list<string>::const_iterator e = objects.begin(); e != objects.end(); e++)
    {
        objectstr += (*e + " ");
    }
    
    return objectstr;
}


bool isCExt(const string& ext)
{
    return (ext == ".c" || ext == ".c86");
}

bool isCPPExt(const string& ext)
{
    return (ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".c++");
}

bool isFORTRANExt(const string& ext)
{
    return (ext == ".f" || ext == ".f77" || ext == ".f90" || ext == ".for");
}

/*
Tells whether the given file name is a source file or not.

Takes: string (file name)
Returns: true if file is a source file
         false if file is not a source file
*/
bool isSourceFile(const string& file)
{
    unsigned int dotpos = file.find_last_of(".");
    if(dotpos != string::npos)
    {
        string ext = file.substr(dotpos, string::npos);
        toLower(ext);
        if(ext == ".c" || ext == ".c86" || ext == ".cpp" || ext == ".cxx" || ext == ".cc")
            return true;
    }
    return false;
}

void checkSourceExistence(list<string>& sources)
{
    for(list<string>::iterator e = sources.begin(); e != sources.end();)
    {
        if(!ioExists(*e))
        {
            UI_warning("%s not found!  pile will ignore it.\n", e->c_str());
            sources.erase(e);
            e = sources.begin();
        }
        else
            e++;
    }
}


int main(int argc, char* argv[])
{
    Environment env;
    Configuration config;
    
    // Create config dir
    ioNewDir(getConfigDir());
    
    // Set log file
    //log_file = getConfigDir() + "/pile_log.txt";
    log_file = "pile_log.txt";
    
    // Delete log file
    ioDelete(log_file.c_str());
    
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
    
    
    bool graphical = false;
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
        else if(string("pile") == ioStripExt(argv[i]))
        {
            // Found a pilefile...
            // Change directory first
            ioSetCWD(ioStripDir(argv[i]));
            file = ioStripFile(argv[i]);
        }
    }
    
    UI_processEvents();
    
    UI_updateScreen();
    
    
    
    if(!graphical)
    {
        UI_init(false, config);
    }
    
    UI_debug_pile("Current directory: %s", ioGetCWD().c_str());
    
    bool changedOutfile = false;
    
    // Find the appropriate pilefile
    if(file == "")
    {
            UI_debug_pile("No pilefile specified.  Searching...\n");
            file = findPileFile();
    }
    
    if(file != "")
    {
        bool errorFlag = false;
        UI_processEvents();
        UI_updateScreen();
        UI_debug_pile("Found pilefile: %s\n", file.c_str());
        if(!interpret(file, env))
            errorFlag = true;
        UI_debug_pile("Done interpreting.\n");
        UI_processEvents();
        UI_updateScreen();
        
        if(!errorFlag)
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
            
            if(!errorFlag)
            {
                UI_debug_pile("Building.\n");
                if(!build(env, config))
                    errorFlag = true;
                if(!errorFlag)
                {
                    UI_debug_pile("Linking.\n");
                    if(!link(config.languages.find("CPP_LINKER_D")->second, env, config))
                        errorFlag = true;
                }
            }
        }
        
        if(errorFlag)
            UI_error("\nBuild errors have occured...\n");
        
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
    
    UI_processEvents();
    UI_updateScreen();
    
    // FIXME: I should fix the auto-naming later...
    changedOutfile = true;
    
    env.print();
    
    bool buildFlag = true;
    
    // Early command-line args
    for(int i = 1; i < argc; i++)
    {
        if(i == 1 && string("edit") == argv[i])
        {
            if(argc > 2)
                file = argv[2];
            if(file == "pile.conf")
            {
                file = configDir + "pile.conf";
            }
            if(file == "")
                file = "com.pile";
            edit(file, config);
            return 0;
        }
        else if(string("--version") == argv[i])
        {
            UI_print("pile Version %s\n", getVersion().c_str());
            if(i == 1 && argc <= 2)
                return 0;
        }
        else if(string("scan") == argv[i])
        {
            buildFlag = false;
        }
        else if(string("build") == argv[i])
        {
            buildFlag = true;
        }
        else if(buildFlag && isSourceFile(argv[i]))
        {
            env.sources.push_back(argv[i]);
            if(!changedOutfile)
            {
                env.outfile = getExeName(argv[i]);
                changedOutfile = true;
            }
            i++;
            for(; i < argc; i++)
            {
                if(string("out:") == argv[i] || string("output:") == argv[i])
                {
                    i++;
                    if(i < argc)
                    {
                        env.outfile = argv[i];
                    }
                }
                else if(isSourceFile(argv[i]))
                    env.sources.push_back(argv[i]);
                else if(firstChar(argv[i], '-'))
                {
                    config.cflags += " " + string(argv[i]);
                    config.lflags += " " + string(argv[i]);
                }
                else
                {
                    config.libraries += " " + string(argv[i]);
                }
            }
            checkSourceExistence(env.sources);
            fflush(stdout);
            
            if(config.useAutoDepend)
            {
                for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
                {
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
                }
            }
            build(env, config);
            link(config.languages.find("CPP_LINKER_D")->second, env, config);
            
            return 0;
        }
        else if(string("out:") == argv[i] || string("output:") == argv[i])
        {
            i++;
            if(i < argc)
            {
                env.outfile = argv[i];
                changedOutfile = true;
            }
        }
    }
    
    UI_processEvents();
    UI_updateScreen();
    
    // Load the pilefile
    if(file != "")
    {
        loadPileFile(file, config, env.outfile, env.sources, env.objects);
        
        // Organize data
        checkSourceExistence(env.sources);
        fflush(stdout);
    }
    
    // No command-line args...  Build it!
    if(argc <= 1)
    {
        if(file == "")
        {
            // Prompt user in order to proceed to build all local source files.
            if(UI_prompt(" No Pilefile found here.  Should I try to build all local source files?\n"))
            {
                env.sources = getLocalSourceFiles();
                config.cflags += UI_promptString(" Compiler flags?\n");
                config.lflags += UI_promptString(" Linker flags?\n");
            }
            else
                return 0;
        }
        
        // Scan for dependencies
        if(config.useAutoDepend)
        {
            for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
            {
                recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
            }
        }
        
        build(env, config);
        link(config.languages.find("CPP_LINKER_D")->second, env, config);
        return 0;
    }
    
    // Parse other command-line args
    for(int i = 1; i < argc; i++)
    {
        if(string("clean") == argv[i])
        {
            bool cleanall = false;
            if(i+1 < argc && string("all") == argv[i+1])
            {
                cleanall = true;
                i++;
                clean(cleanall, env.sources, config, env.outfile);
            }
            else if(i+1 < argc && (string("o") == argv[i+1] || string(".o") == argv[i+1]))
            {
                i++;
                clean(".o");
            }
            else
            {
                clean(cleanall, env.sources, config, env.outfile);
            }
        }
        else if(string("clean.o") == argv[i])
        {
            clean(".o");
        }
        else if(string("scan") == argv[i])
        {
            bool checkedOne = false;
            i++;
            for(; i < argc; i++)
            {
                if(isSourceFile(argv[i]))
                {
                    checkedOne = true;
                    //printDepends(argv[i]);
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, argv[i], "");
                    //UI_debug_pile("Recursed, got %d depends.\n", depends.size());
                }
                else
                {
                    i--;
                    break;
                }
            }
            if(!checkedOne)
            {
                for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
                {
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
                }
                //UI_debug_pile("Recursed, got %d depends.\n", depends.size());
            }
        }
        else if(string("build") == argv[i])
        {
            if(config.useAutoDepend)
            {
                for(list<string>::iterator e = env.sources.begin(); e != env.sources.end(); e++)
                {
                    recurseIncludes(env.depends, env.fileDataHash, config.includePaths, *e, "");
                }
            }
            build(env, config);
            link(config.languages.find("CPP_LINKER_D")->second, env, config);
        }
        else if(string("-g") == argv[i] || string("pile") == ioStripExt(argv[i]))
        {}
        else
        {
            UI_warning("pile Warning: Command \"%s\" not found, so it will be ignored.\n", argv[i]);
        }
    }
    
    
    if(graphical)
    {
        ui_print = true;
        ui_log_print = false;
        UI_print("\nAll done!  Press any key.\n");
        UI_updateScreen();
        
        UI_waitKeyPress();
    }
    
    UI_quit();
    
    return 0;
}
