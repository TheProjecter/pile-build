/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_config.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains the functions which parse the configuration file.
*/

#include "pile_global.h"
#include "Eve Source/eve_interpreter.h"
#include "External Code/goodio.h"
#include "pile_env.h"
#include "pile_config.h"
#include "pile_ui.h"
#include "string_functions.h"
#include <fstream>

extern Interpreter interpreter;
extern Environment env;
bool interpret(string filename, Environment& env, Configuration& config);
bool interpretNoPile(string filename, Environment& env, Configuration& config);

string getVersion();

string strip(string str, char c);
bool isWhitespace(const char& c);
bool isMatch(const char& c, const vector<char>& matching);
bool isQuantizer(const char& c);
string nextToken(string& line);
list<string> tokenize(string& line);

char removeQuantifiers(string& str);


/*
Creates a new config file with default values which depend on the operating system.

Takes: string (Config file location)
       Configuration (config settings)
Returns: true on success
         false on failure
*/
bool createConfig(string path, Configuration& config)
{
    UI_log("Creating config\n");
    ofstream fout((path + "template_pile.conf").c_str(), ios::trunc);
    if(fout.fail())
    {
    UI_log("Failed.\n");
        fout.close();
        return false;
    }

    UI_log("Writing config\n");
    fout << "CONFIG_FORMAT_VERSION_MAJOR = " << VERSION_MAJOR << endl;
    fout << "CONFIG_FORMAT_VERSION_MINOR = " << VERSION_MINOR << endl;
    fout << "CONFIG_FORMAT_VERSION_BUGFIX = " << VERSION_BUGFIX << endl;
    UI_log("Wrote version\n");

    UI_log("Writing editor\n");
    if(config.editor == "vi")
    {
        fout << "// Notice: The default editor is 'vi'." << endl
             << "//  If you don't have experience with vi, it is recommended that" << endl
             << "//  you change the default editor." << endl
             << "//  vi controls:" << endl
             << "//    Press 'esc' to make sure that you're in Command mode" << endl
             << "//    Quitting: Type :q to quit or type :x to save and quit." << endl
             << "//    Deleting: Press 'x' or 'X' to delete characters." << endl
             << "//    Deleting: Press 'dd' to delete a line." << endl
             << "//    Editing: Press 'i' to enter Insert mode." << endl << endl;
        UI_log("Wrote vi warning\n");
    }

    fout << "EDITOR_PATH = " << quoteThis(config.editor) << endl;
    UI_log("Wrote editor\n");

    fout << "PILE_PATH = " << quoteThis(config.installPath) << endl;

    /*fout << "lang C: " << config.languages.find("C_COMPILER")->second << ", "
                  << config.languages.find("C_LINKER_D")->second << ", "
                  << config.languages.find("C_LINKER_S")->second << ", "
                  << config.languages.find("C_SYNTAX")->second << endl;*/
    fout << "cpp_compiler.name = " << quoteThis(config.languages.find("CPP_COMPILER")->second) << endl;
    fout << "cpp_compiler.path = " << quoteThis(config.languages.find("CPP_COMPILER")->second) << endl;
    fout << "cpp_linker.name = " << quoteThis(config.languages.find("CPP_LINKER_D")->second) << endl;
    fout << "cpp_linker.path = " << quoteThis(config.languages.find("CPP_LINKER_D")->second) << endl;
    /*fout << "lang FORTRAN: " << config.languages.find("FORTRAN_COMPILER")->second << ", "
                  << config.languages.find("FORTRAN_LINKER_D")->second << ", "
                  << config.languages.find("FORTRAN_LINKER_S")->second << ", "
                  << config.languages.find("FORTRAN_SYNTAX")->second << endl;*/
    fout << "BIN_INSTALL_DIR = " << quoteThis(config.binInstallPath) << endl;
    fout << "PROGRAM_INSTALL_DIR = " << quoteThis(config.programInstallPath) << endl;
    fout << "LIBRARY_INSTALL_DIR = " << quoteThis(config.libInstallPath) << endl;
    fout << "HEADER_INSTALL_DIR = " << quoteThis(config.headerInstallPath) << endl;

    /*fout << "includeDirs:";
    for(list<string>::iterator e = config.includePaths.begin(); e != config.includePaths.end(); e++)
    {
        fout << " " << *e;
    }
    fout << endl;*/

    UI_log("Done writing config\n");
    fout.close();
    return true;
}


/*
Fills in the lists of language, compiler, and linkers from the config file.
Also creates a config file if none exists.

Takes: string (config file location)
       Configuration (config settings)
Returns: true on success
         false on failure
*/
bool loadConfig(string path, Configuration& config)
{
    /*
    Pile has three files that it deals with for configuration.
    pile.conf is the config file, in the ~/.pile directory.
    template_pile.conf is the template for the config file, in the ~/.pile directory.
    template_pile.conf is the template for the config file, in Pile's installation directory.

    If pile.conf does not exist, Pile will copy the local template (the one in
    ~/.pile.  This allows you to make changes to the template, which will be the
    default if pile.conf is deleted.  If that template does not exist, then Pile
    copies the one in its installation directory.  If that doesn't work, Pile
    tries to create a new template file in ~/.pile, assuming that the user does
    not have write access to Pile's installation directory.
    */
    string file = path + "pile.conf";
    if(!ioExists(path + "template_pile.conf"))
    {
        UI_warning("pile config file template not found.\n");
        if(ioCopy("template_pile.conf", file))
        {
            UI_warning("Failed to copy Pile's config file template.\n");
        }
        else if(createConfig(path, config))
            UI_warning("Created template config file successfully.\n");
        else
        {
            UI_error("pile error: Failed to create template config file.\n");
            return false;
        }
    }

    if(!ioExists(file))
    {
        UI_warning("Config file does not exist.  Copying template...\n");
        if(ioCopy(path + "template_pile.conf", file) || ioCopy("template_pile.conf", file))
            UI_warning("Copied config file successfully.\n");
        else
        {
            UI_error("pile error: Failed to copy config file.\n");
            return false;
        }
    }

    Scope& s = *(interpreter.env.begin());

    Int* version_major = new Int("<temp>", VERSION_MAJOR);
    version_major->reference = true;
    Int* version_minor = new Int("<temp>", VERSION_MINOR);
    version_minor->reference = true;
    Int* version_bugfix = new Int("<temp>", VERSION_BUGFIX);
    version_bugfix->reference = true;

    String* editor_path = new String("EDITOR_PATH", "vi");
    editor_path->reference = true;
    String* pile_path = new String("PILE_PATH", config.installPath);
    pile_path->reference = true;
    String* bin_install_path = new String("BIN_INSTALL_DIR", config.binInstallPath);
    bin_install_path->reference = true;
    String* program_install_path = new String("PROGRAM_INSTALL_DIR", config.programInstallPath);
    program_install_path->reference = true;
    String* lib_install_path = new String("LIBRARY_INSTALL_DIR", config.libInstallPath);
    lib_install_path->reference = true;
    String* header_install_path = new String("HEADER_INSTALL_DIR", config.headerInstallPath);
    header_install_path->reference = true;

    s.env["CONFIG_FORMAT_VERSION_MAJOR"] = version_major;
    s.env["CONFIG_FORMAT_VERSION_MINOR"] = version_minor;
    s.env["CONFIG_FORMAT_VERSION_BUGFIX"] = version_bugfix;

    s.env["EDITOR_PATH"] = editor_path;
    s.env["PILE_PATH"] = pile_path;


    s.env["BIN_INSTALL_DIR"] = bin_install_path;
    s.env["PROGRAM_INSTALL_DIR"] = program_install_path;
    s.env["LIBRARY_INSTALL_DIR"] = lib_install_path;
    s.env["HEADER_INSTALL_DIR"] = header_install_path;


    // Add Compiler
    Class* compiler = new Class("Compiler");
    compiler->addVariable("string", "name");
    compiler->addVariable("string", "path");
    interpreter.addClass(compiler);

    ClassObject* cpp_compiler = new ClassObject("cpp_compiler", "Compiler");
    cpp_compiler->reference = true;
    Variable* cpp_name = cpp_compiler->getVariable("name");
    Variable* cpp_path = cpp_compiler->getVariable("path");
    if(cpp_name != NULL && cpp_path != NULL && cpp_name->getType() == STRING && cpp_path->getType() == STRING)
    {
        static_cast<String*>(cpp_name)->setValue(config.languages["CPP_COMPILER"]);
        static_cast<String*>(cpp_path)->setValue(config.languages["CPP_COMPILER"]);
    }
    s.env["cpp_compiler"] = cpp_compiler;

    // Add Linker
    Class* linker = new Class("Linker");
    linker->addVariable("string", "name");
    linker->addVariable("string", "path");
    interpreter.addClass(linker);

    ClassObject* cpp_linker = new ClassObject("cpp_linker", "Linker");
    cpp_linker->reference = true;
    Variable* cpp_linkname = cpp_linker->getVariable("name");
    Variable* cpp_linkpath = cpp_linker->getVariable("path");
    if(cpp_linkname != NULL && cpp_linkpath != NULL && cpp_linkname->getType() == STRING && cpp_linkpath->getType() == STRING)
    {
        static_cast<String*>(cpp_linkname)->setValue(config.languages["CPP_LINKER_D"]);
        static_cast<String*>(cpp_linkpath)->setValue(config.languages["CPP_LINKER_D"]);
    }
    s.env["cpp_linker"] = cpp_linker;



    interpreter.allowDeclarations = false;
    // Do the interpretation
    if(!interpretNoPile(file, env, config))
    {
        UI_error("An error occurred while reading config file.\n");
        interpreter.allowDeclarations = true;
        interpreter.reset();
        return false;
    }
    else
    {
        interpreter.allowDeclarations = true;

        // Retrieve the variables
        config.editor = editor_path->getValue();
        config.installPath = pile_path->getValue();

        // FIXME: This needs to be much better and flexible
        config.languages["CPP_COMPILER"] = static_cast<String*>(cpp_path)->getValue();
        config.languages["CPP_LINKER_D"] = static_cast<String*>(cpp_linkpath)->getValue();

        config.binInstallPath = bin_install_path->getValue();
        config.programInstallPath = program_install_path->getValue();
        config.libInstallPath = lib_install_path->getValue();
        config.headerInstallPath = header_install_path->getValue();

        interpreter.reset();
    }
    return true;
}
