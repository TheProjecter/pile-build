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


void generatePilefile(string name)
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
    ioAppend("output = \"" + base + "\"\nsources += [\"main.cpp\"]\n//lflags += [\"-lSDLmain\", \"-lSDL\"]\n//cflags += [\"`sdl-config --cflags`\"]", name);
}


int main(int argc, char* argv[])
{
    Environment env;
    Configuration config;
    
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
    bool dryRun = false;  // Interpret without actions, but with messages
    bool noLink = false;
    bool noCompile = false;
    bool changedOutfile = false;
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
        else if(string("new") == argv[i])
        {
            // Generate a new pilefile
            string name = "com.pile";
            i++;
            if(i < argc)
            {
                name = argv[i];
            }
            
            generatePilefile(name);
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
            dryRun = true;
        }
        else if(string("--nocompile") == argv[i])
        {
            noCompile = true;
        }
        else if(string("--nolink") == argv[i])
        {
            noLink = true;
        }
        // FIXME: Change this to check for substring 'out=', grab the file name, then strip the (optional) quotes.
        else if(string("out:") == argv[i] || string("output:") == argv[i])
        {
            i++;
            if(i < argc)
            {
                env.outfile = argv[i];
                changedOutfile = true;
            }
        }
        // Check for .pile file extension ('pile myfile.pile')
        else if(string("pile") == ioStripToExt(argv[i]))
        {
            // Found a pilefile...
            // Change directory first
            ioSetCWD(ioStripToDir(argv[i]));
            file = ioStripToFile(argv[i]);
        }
        else
        {
            UI_warning("pile Warning: Command \"%s\" not found, so it will be ignored.\n", argv[i]);
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
        
        if(!interpret(file, env))
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
            
            if(!dryRun && !cleaning && !errorFlag)
            {
                UI_debug_pile("Building and linking.\n");
                if(!noCompile && !build(env, config))
                    errorFlag = true;
                if(!errorFlag)
                    if(!noLink && !link(config.languages.find("CPP_LINKER_D")->second, env, config))
                        errorFlag = true;
            }
        }
        
    }
    else  // No Pilefile found
    {
        // Prompt user in order to proceed to build all local source files.
        if(UI_prompt(" No Pilefile found here.  Should I try to build all local source files?\n"))
        {
            env.sources = getLocalSourceFiles();
            config.cflags += UI_promptString(" Compiler flags?\n");
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
            if(!dryRun && !cleaning && !errorFlag)
            {
                if(!noCompile && !build(env, config))
                    errorFlag = true;
                if(!errorFlag && !noLink && !link(config.languages.find("CPP_LINKER_D")->second, env, config))
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
    
    //env.print();
    //FIXME: Should I use this? -> checkSourceExistence(env.sources);
    
    // Parse other command-line args
    /*for(int i = 1; i < argc; i++)
    {
        if(string("scan") == argv[i])
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
    }*/
}
