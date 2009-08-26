/*
goodIO - A small library for simple filesystem interaction.
by Jonathan Dearborn, 2009
grimfang4 [at] gmail [dot] com

See goodio.h for details and license.
*/

#include "goodio.h"

#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>

#define IO_COPY_BUFFSIZE 512
#define IO_UNIQUE_MAX 10000

using namespace std;

bool ioExists(string filename)
{
    return (access(filename.c_str(), 0) == 0);
}

bool ioIsDir(string filename)
{
    struct stat status;
    stat(filename.c_str(), &status);

    return (status.st_mode & S_IFDIR);
}

bool ioIsFile(string filename)
{
    struct stat status;
    stat(filename.c_str(), &status);

    return (status.st_mode & S_IFREG);
}

bool ioIsReadable(string filename)
{
    return (access(filename.c_str(), 4) == 0);
}

bool ioIsWriteable(string filename)
{
    return (access(filename.c_str(), 2) == 0);
}

bool ioIsReadWriteable(string filename)
{
    return (access(filename.c_str(), 6) == 0);
}

bool ioNew(string filename, bool readable, bool writeable)
{
    if(ioExists(filename))
        return false;
    
    FILE* file = fopen(filename.c_str(), "wb");
    if(file == NULL)
        return false;
    fclose(file);
    return true;
}

bool ioNewDir(string dirname, int mode)
{
    if(ioExists(dirname))
        return false;
    
    #ifdef WIN32
    return (mkdir(dirname.c_str()) == 0);
    #else
    int userBits =   (mode & IO_USER?   (mode & IO_READ? S_IXUSR | S_IRUSR : 0) | (mode & IO_WRITE? S_IWUSR : 0) : 0);
    int groupBits =  (mode & IO_GROUP?  (mode & IO_READ? S_IXGRP | S_IRGRP : 0) | (mode & IO_WRITE? S_IWGRP : 0) : 0);
    int othersBits = (mode & IO_OTHERS? (mode & IO_READ? S_IXOTH | S_IROTH : 0) | (mode & IO_WRITE? S_IWOTH : 0) : 0);
    
    return (mkdir(dirname.c_str(), userBits | groupBits | othersBits) == 0);
    #endif
}

bool ioDelete(string filename)
{
    #ifdef WIN32
    return (unlink(filename.c_str()) == 0) || (rmdir(filename.c_str()) == 0);
    #else
    return (remove(filename.c_str()) == 0);
    #endif
}

bool ioMove(string source, string dest)
{
    if(source == dest)
        return true;
    
    if(!ioCopy(source, dest))
        return false;
    
    if(!ioDelete(source))
        return false;
    
    return true;
}

// This has a same device limitation
bool ioRename(string source, string dest)
{
    return (rename(source.c_str(), dest.c_str()) == 0);
}

bool ioCopy(string source, string dest)
{
    if(source == dest)
        return true;
    
    FILE* src = fopen(source.c_str(), "rb");
    if(src == NULL)
        return false;
    
    FILE* dst = fopen(dest.c_str(), "wb");
    if(dst == NULL)
    {
        fclose(src);
        return false;
    }
    
    char buff[IO_COPY_BUFFSIZE];
    unsigned int numBytes;

    while((numBytes = fread(buff, 1, IO_COPY_BUFFSIZE, src)) > 0)
    {
        if(fwrite(buff, 1, numBytes, dst) != numBytes)
        {
            fclose(src);
            fclose(dst);
            return false;
        }
    }

    fclose(src);
    fclose(dst);

    return true;
}

bool ioClear(std::string filename)
{
    if(!ioExists(filename))
        return false;
    
    FILE* file = fopen(filename.c_str(), "wb");
    if(file == NULL)
        return false;
    fclose(file);
    return true;
}

// Should these have binary counterparts?
// Prepend avoids access permissions, but append respects them...  Oh well!
bool ioPrepend(std::string text, std::string filename)
{
    // Create a temp file
    string temp = ioUniqueName(filename, 0);
    if(temp == "" || !ioNew(temp))
        return false;
    
    // Put the new stuff into temp
    if(!ioAppend(text, temp))
    {
        ioDelete(temp);
        return false;
    }
    
    // It may be safer to use just one open/close per file instead of using goodIO:
    //Now use ioCopy's code to append all of the source file to the temp file.
    //ioDelete(filename);
    //FILE* result = fopen(filename.c_str(), "w");
    //copy all of the temp file data to the new file now.
    //ioDelete(temp);
    
    // Put the old stuff into temp
    if(!ioAppendFile(filename, temp))
    {
        ioDelete(temp);
        return false;
    }
    
    // Replace the old file
    ioDelete(filename);
    if(!ioRename(temp, filename))
        return false;
    
    return true;
}

bool ioAppend(string text, string filename)
{
    FILE* file = fopen(filename.c_str(), "a");
    if(file == NULL)
        return false;

    if(fwrite(text.c_str(), 1, text.length(), file) != text.length())
    {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

bool ioAppendFile(string srcfile, string destfile)
{
    FILE* file1 = fopen(destfile.c_str(), "a");
    if(file1 == NULL)
        return false;
    FILE* file2 = fopen(srcfile.c_str(), "r");
    if(file2 == NULL)
    {
        fclose(file1);
        return false;
    }

    char buff[IO_COPY_BUFFSIZE];
    unsigned int numBytes;

    while((numBytes = fread(buff, 1, IO_COPY_BUFFSIZE, file2)) > 0)
    {
        if(fwrite(buff, 1, numBytes, file1) != numBytes)
        {
            fclose(file1);
            fclose(file2);
            return false;
        }
    }

    fclose(file1);
    fclose(file2);
    return true;
}

string ioStripDir(string filename)
{
    size_t lastSlash = filename.find_last_of("/\\");
    if(lastSlash == string::npos)
        return "";
    return filename.substr(0, lastSlash);
}

string ioStripFile(string filename)
{
    return filename.substr(filename.find_last_of("/\\") + 1);  // If npos, it should become 0, which is good :)
}

string ioStripExt(string filename)
{
    size_t lastDot = filename.find_last_of(".");
    size_t lastSlash = filename.find_last_of("/\\");
    if(lastDot == string::npos || lastSlash >= lastDot - 1) // "/usr/cow" or "/usr/.hidden" or "/usr/tex.d/"
        return "";
    return filename.substr(lastDot + 1);
}

string ioUniqueName(string filename, int startAt)
{
    string ext, base;
    size_t lastDot = filename.find_last_of(".");
    size_t lastSlash = filename.find_last_of("/\\");
    if(lastDot == string::npos || (lastSlash != string::npos && lastSlash >= lastDot))
    {
        ext = "";
        base = filename;
    }
    else
    {
        ext = filename.substr(lastDot);
        base = filename.substr(0, lastDot);
    }
    
    char buff[20];
    for(; startAt < IO_UNIQUE_MAX; startAt++)
    {
        sprintf(buff, "%d", startAt);
        if(!ioExists(base + buff + ext))
            return base + buff + ext;
    }
    
    return "";
}

int ioSize(string filename)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return -1;
    return status.st_size;
}

time_t ioTimeAccessed(string filename)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return -1;
    return status.st_atime;
}

time_t ioTimeModified(string filename)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return -1;
    return status.st_mtime;
}

time_t ioTimeStatus(string filename)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return -1;
    return status.st_ctime;
}

string ioTimeString(time_t time)
{
    return ctime(&time);
}

bool ioSetReadable(string filename, bool readable)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return false;
    
    // Preserve the other permission
    int writeBit = status.st_mode & S_IWRITE;
    if(readable)
    {
        if(status.st_mode & S_IREAD)
            return true;
        return (chmod(filename.c_str(), writeBit | S_IREAD) == 0);
    }
    else
    {
        if(!(status.st_mode & S_IREAD))
            return true;
        return (chmod(filename.c_str(), writeBit) == 0);
    }
}

bool ioSetWriteable(string filename, bool writeable)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return false;
    
    // Preserve the other permission
    int readBit = status.st_mode & S_IREAD;
    if(writeable)
    {
        if(status.st_mode & S_IWRITE)
            return true;
        return (chmod(filename.c_str(), readBit | S_IWRITE) == 0);
    }
    else
    {
        if(!(status.st_mode & S_IWRITE))
            return true;
        return (chmod(filename.c_str(), readBit) == 0);
    }
}

bool ioSetReadWriteable(string filename, bool ReadWriteable)
{
    struct stat status;
    if(stat(filename.c_str(), &status) < 0)
        return false;

    return (chmod(filename.c_str(), ReadWriteable? S_IREAD | S_IWRITE : 0) == 0);
}


string ioGetProgramDir(string argv0)
{
    string path = argv0;
    path = path.substr(0, path.find_last_of('/'));  // Remove exe name from string
    
    // Maybe I should compare the cwd with path to get the result?
    //string cwd = getcwd(NULL, 0);
    // They may overlap and be absolute (Code::Blocks): cwd == "/usr/bin", path == "/usr/bin/myapp"
    // They may be distinct (Konqueror): cwd == "/home/myself", path == "/usr/bin/myapp"
    // They may overlap, but be relative (Terminal): cwd == "/usr/bin", path == "./myapp"
    // Similarly (Terminal): cwd == "/usr", path == "./bin/myapp"
    // They may be totally non-helpful! (e.g. installed in /bin): cwd == "/home/myself/stuff", path == "myapp"
    return path;
}

bool ioResetCWD(string argv0)
{
    return (chdir(ioGetProgramDir(argv0).c_str()) == 0);
}

bool ioSetCWD(string dir)
{
    return (chdir(dir.c_str()) == 0);
}

string ioGetCWD()
{
    char s[PATH_MAX];
    return getcwd(s, PATH_MAX);
}


list<string> ioList(string dirname, bool directories, bool files)
{
    list<string> dirList;
    list<string> fileList;
    
    DIR* dir = opendir(dirname.c_str());
    dirent* entry;
    
    while ((entry = readdir(dir)) != NULL)
    {
        #ifdef WIN32
        if(ioIsDir(dirname + "/" + entry->d_name))
        #else
        if(entry->d_type == DT_DIR)
        #endif
        {
            if(directories)
                dirList.push_back(entry->d_name);
        }
        else if(files)
            fileList.push_back(entry->d_name);
    }
 
    closedir(dir);
    
    dirList.sort();
    fileList.sort();
    
    fileList.splice(fileList.begin(), dirList);
    
    
    return fileList;
}
