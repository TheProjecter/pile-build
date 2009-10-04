/*
goodIO v1.0 - A small library for simple filesystem interaction.
by Jonathan Dearborn 3-28-09
grimfang4 [at] gmail [dot] com

The goodIO functions are as follows:

string ioGetProgramDirectory(string argv0);  // Returns the directory of the executable
bool ioResetCWD(string argv0);  // Changes the current working directory to the result of ioGetProgramDirectory()
bool ioResetCWDm();  // Macro version of ioResetCWD(), can only be used when argv[] is in scope (i.e. in main())
bool ioExists(string filename);  // Does the file exist?
bool ioIsDir(string filename);  // Is the file a directory?
bool ioIsFile(string filename);  // Is the file a regular file (not a directory)?
bool ioIsHidden(string filename);  // Is the file hidden?
int ioSize(string filename);  // Get the size of a file in bytes
int ioTimeAccessed(string filename);  // Get the time a file was last accessed
int ioTimeModified(string filename);  // Get the time a file was last modified
int ioTimeStatus(string filename);  // Get the time a file last had its status (file properties) changed
string ioTimeString(time_t time);  // Convert the result of a goodIO time function into a human-readable string
list<string> ioList(string dirname, bool directories = true, bool files = true);  // Returns a list of files
string ioStripDir(string filename);  // Get the parent directory
string ioStripFile(string filename);  // Get the file name without the path
string ioStripExt(string filename);  // Get the file extension (type)
string ioUniqueName(string filename, int startAt);  // Get an unused file name (e.g. file.txt -> file5.txt)
bool ioIsReadable(string filename);  // Do I have 'read' permissions for this file?
bool ioIsWriteable(string filename);  // Do I have 'write' permissions for this file?
bool ioIsReadWriteable(string filename);  // Do I have 'read' and 'write' permissions for this file?
bool ioSetReadable(string filename, bool readable = true);  // Change 'read' permissions
bool ioSetWriteable(string filename, bool writeable = true);  // Change 'write' permissions
bool ioSetReadWriteable(string filename, bool ReadWriteable = true);  // Change 'read' and 'write' permissions
bool ioNew(string filename, bool readable = true, bool writeable = true);  // Create an empty file
bool ioNewDir(string dirname, int mode = IO_USER | IO_READWRITE);  // Create an empty directory (see Notes below)
bool ioDelete(string filename);  // Delete a file
bool ioMove(string source, string dest);  // Move a file
bool ioRename(string source, string dest);  // Rename a file
bool ioCopy(string source, string dest);  // Copy a file
bool ioClear(string filename);  // Erase the contents of a file
bool ioAppend(string filename, string text);  // Add text to the end of a file's contents
bool ioPrepend(string text, string filename);  // Add text to the beginning of a file's contents
bool ioAppendFile(string srcfile, string destfile);  // Concatenate two files

Notes:

Functions which return a 'bool' will return 'true' on success and 'false' on failure.
Functions which return an 'int' will return a value >= 0 on success and -1 on failure.
Your main() should be declared as:
        int main(int argc, char* argv[])
    Then you can use ioResetCWD(argv[0]) or ioResetCWDm()
    ioResetCWD() is provided for cases when your executable is launched in a strange way
    that causes the current working directory to be something other than what your
    program expects (i.e. launching from a different directory or from Konqueror).
    It does not, however, fix this if the executable is installed in a directory that, 
    for instance, the command shell searches by default (e.g. /bin for BASH).
The privilege flags available to ioNewDir() are:
    IO_NONE, IO_USER, IO_GROUP, IO_OTHERS  // Who can do it
    IO_NONE, IO_READ, IO_WRITE, IO_READWRITE  // What they can do
    e.g. IO_USER | IO_GROUP | IO_WRITE  // Gives write access to the user and group



goodIO is released under the TSL:
    The short:
    Use it however you'd like, but give me credit if you share this code
    or a compiled version of the code, whether or not it is modified.
    
    The long:
    Copyright (c) 2009, Jonathan Dearborn
    All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list 
      of conditions and the following disclaimer.
    * Redistributions in binary form, excluding commercial executables, must reproduce 
      the above copyright notice, this list of conditions and the following disclaimer 
      in the documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote products 
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY 
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _GOODIO_H__
#define _GOODIO_H__

#include <string>
#include <list>
#include <fstream>


#define IO_NONE 0
#define IO_READ 1
#define IO_WRITE 2
#define IO_USER 4
#define IO_GROUP 8
#define IO_OTHERS 16

#define IO_READWRITE (IO_READ | IO_WRITE)


extern "C"
{
    
// Path stuff
std::string ioGetProgramDir(std::string argv0);

bool ioResetCWD(std::string argv0);

#define ioResetCWDm() ioResetCWD(argv[0])

bool ioSetCWD(std::string dir);

std::string ioGetCWD();

std::string ioStripDir(std::string filename);

std::string ioStripFile(std::string filename);

std::string ioStripExt(std::string filename);

std::string ioUniqueName(std::string filename, int startAt = 1);

// File info
bool ioExists(std::string filename);

bool ioIsDir(std::string filename);

bool ioIsFile(std::string filename);

int ioSize(std::string filename);

time_t ioTimeAccessed(std::string filename);

time_t ioTimeModified(std::string filename);

time_t ioTimeStatus(std::string filename);

std::string ioTimeString(time_t time);


// Access testing
bool ioIsReadable(std::string filename);

bool ioIsWriteable(std::string filename);

bool ioIsReadWriteable(std::string filename);

// Access setting
bool ioSetReadable(std::string filename, bool readable = true);

bool ioSetWriteable(std::string filename, bool writeable = true);

bool ioSetReadWriteable(std::string filename, bool ReadWriteable = true);


// File modification
bool ioNew(std::string filename, bool readable = true, bool writeable = true);

bool ioNewDir(std::string dirname, int mode = IO_USER | IO_READWRITE);

bool ioDelete(std::string filename);

bool ioMove(std::string source, std::string dest);

bool ioCopy(std::string source, std::string dest);

bool ioRename(std::string source, std::string dest);

bool ioClear(std::string filename);

// File writing

bool ioAppend(std::string text, std::string filename);

bool ioPrepend(std::string text, std::string filename);

bool ioAppendFile(std::string srcfile, std::string destfile);


// Directory listing
std::list<std::string> ioList(std::string dirname, bool directories = true, bool files = true);

std::list<std::string> ioExplode(std::string str, char delimiter = ' ');

class ioFileReader
{
private:
    std::ifstream fin;
    std::string file;
 
    ioFileReader (const ioFileReader&);
    ioFileReader& operator=(const ioFileReader&);
    
    void stripCarriageReturn(std::string& str);

public:
    ioFileReader(const std::string& filename);
 
    ~ioFileReader();
    
    char get();
    
    char peek();
 
    std::string getLine();
 
    std::string getString(char delimiter);
 
    std::string getStringUntil(char delimiter);
 
    bool getBool();
 
    int getInt();
 
    unsigned int getUInt();
 
    long getLong();
 
    unsigned long getULong();
 
    float getFloat();
 
    double getDouble();
    
    // Ignore the next characters
    void skip(unsigned int numChars);
    
    void skip(unsigned int numChars, char delimiter);
    
    void skip(char delimiter);
    
    void skipUntil(unsigned int numChars, char delimiter);
    
    void skipUntil(char delimiter);
    
    void skipLine();
    
    bool ready();
};


}


#endif
