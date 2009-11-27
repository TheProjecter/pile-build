/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_depend.h

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

Header for pile_depend.cpp, contains FileData class definition.

*/

#ifndef _PILE_DEPEND_H__
#define _PILE_DEPEND_H__

#include <string>
#include "External Code/goodio.h"

class FileData
{
    private:
    bool fileExists;
    std::string fullPath;  // includes file name
    std::string filename;
    
    //string checksum;
    time_t modifiedTime;
    time_t dependTime;
    
    public:
    
    FileData()
        : fileExists(false)
        , modifiedTime(0)
        , dependTime(0)
    {}
    
    FileData(const std::string& fullPath)
        : fileExists(false)
        , fullPath(fullPath)
        , modifiedTime(0)
        , dependTime(0)
    {
        setFileNameFromPath();
        checkExistence();
        if(exists())
        {
            checkTime();
        }
    }
    
    // Setters
    
    void setPath(const std::string& path)
    {
        fullPath = path;
    }
    
    void setFileName(const std::string& fileName)
    {
        filename = fileName;
    }
    
    void setFileNameFromPath()
    {
        unsigned int pos = fullPath.find_last_of("/");
        if(pos != std::string::npos)
            filename = fullPath.substr(pos+1, std::string::npos);
        else
            filename = fullPath;
    }
    
    void setDependTime(time_t newTime)
    {
        dependTime = newTime;
    }
    
    void checkExistence()
    {
        fileExists = ioExists(fullPath);
    }
    
    void checkTime()
    {
        modifiedTime = ioTimeModified(fullPath);
        dependTime = modifiedTime;
    }
    
    
    
    
    // Getters
    
    bool exists()
    {
        return fileExists;
    }
    
    time_t getTime()
    {
        return modifiedTime;
    }
    
    time_t getDependTime()
    {
        return dependTime;
    }
    
    std::string getPath()
    {
        return fullPath;
    }
    
    std::string getFileName()
    {
        return filename;
    }
    
};


void recurseIncludes(std::map<FileData*, std::list<FileData*> >& depends, std::map<std::string, FileData*>& fileDataHash, const std::list<std::string>& paths, const std::string& file, std::string path);

bool mustRebuild(const std::string& objName, std::map<FileData*, std::list<FileData*> > depends, FileData* file);


#endif
