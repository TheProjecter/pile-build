#include "pile_global.h"
#include "pile_depend.h"
#include "pile_ui.h"
#include <fstream>

string getFilePath(string file);
bool isWhitespace(const char& c);
void removePath(string& file);

/*
The strategy:
*Get list of source files.

SCAN:
*For each file, look through it and grab #includes.
*Put those into a list.
Format them (remove whitespace and compress ./ and ../) and put them in a list.

WRITE:
Put dependency info into the pilefile, overwriting existing 'auto' fields.
Do this before LOAD, so that only the auto stuff is in auto.

LOAD:
Get explicit and auto dependencies from the pilefile.
Remove explicit exceptions.

CHECK:
*When building, compare each source file with its dependencies.  If the dependency has a newer time stamp than the source's object file or the object file does not exist, then you must build the source file.

*I could use a map, with the headers as the key and dependents inside a list<string>.
I could store the time for each object too, so I would use list<time_object>, with time_object as a struct that contains a filename and a time.
Iterate through the keys, store the ioTimeModified() of it, then check the time of each object.





Gathering:
*Get the source file names.
Erase any 'auto' lines in the pilefile.
*Scan each file for includes.
*Store these includes as keys with the source file as an entry.
*Scan each include for includes, adding the dependent include as an entry to the other.
Check to see that each source file and header exists.  Warn if a source file does not
exist.  If a header can not be found, then it should be removed from the dependency map.
In this way, I don't have to worry about std includes or PATH.

I need to check #defines and #ifdefs so that I don't get into an infinite loop
over headers that include other headers.
#includes are local to the disk location of the file.
I do need to check the std includes... :(
We'll see how fast it is.  Also, I need to save the name and modification time (or checksum) of each library that is linked (for static only?).  Then if the object files change or a library changes, I can relink.

bool depend(list<string>& sources, string pilefile)
{
    eraseAuto(pilefile);
    
    map<string, list<string> > depends;
    
    foreach(source in sources)
    {
        recurseIncludes(source);
    }
}

Building:
*Iterate through the map of includes.
*For each item, check its modified time.  If it's a source file, then
check the time against its object file's time.  If the object file is older than
the header, then rebuild it.  Remove the entry from the map.  If it is a header
file, then remove the entry from the map and check the new header's key with the first one's time stamp.

*/















void removeUpTo(string& str, char c)
{
    //UI_debug_pile("Removing up to: \"%s\"\n", str.c_str());
    for(string::iterator e = str.begin(); e != str.end();)
    {
        if(*e != c)
        {
            str.erase(e);
            e = str.begin();
        }
        else
            break;
    }
    //UI_debug_pile("Removed\n");
}

void removeBackTo(string& str, char c)
{
    //UI_debug_pile("Removing back to: \"%s\", %c", str.c_str(), c);
    unsigned int pos = str.find_last_of(c);
    if(pos != string::npos)
        str = str.substr(0, pos + 1);
    else
        str.clear();
    //UI_debug_pile("Removed\n");
}

char removeQuantifiers(string& str)
{
    unsigned int quote = str.find_first_of('\"');
    unsigned int angle = str.find_first_of('<');
    char result = '\"';
    
    if(quote == string::npos)
    {
        removeUpTo(str, '<');
        removeBackTo(str, '>');
        result = '<';
    }
    else if(angle == string::npos)
    {
        removeUpTo(str, '\"');
        removeBackTo(str, '\"');
        result = '\"';
    }
    else
    {
        if(quote < angle)
        {
            removeUpTo(str, '\"');
            removeBackTo(str, '\"');
            result = '\"';
        }
        else
        {
            removeUpTo(str, '<');
            removeBackTo(str, '>');
            result = '<';
        }
    }
    
    if(str.size() > 2)
    {
        //UI_debug_pile("Chopping: %s\n", str.c_str());
        str = str.substr(1, str.size()-2);
        //UI_debug_pile("Chopped\n");
    }
    return result;
}

void print_list(const list<string>& ls)
{
    for(list<string>::const_iterator e = ls.begin(); e != ls.end(); e++)
    {
        UI_print("%s\n", e->c_str());
    }
}

list<string> explode(string str, char c)
{
    list<string> result;
    
    unsigned int oldPos = 0;
    unsigned int pos = str.find_first_of(c);
    while(pos != string::npos)
    {
        result.push_back(str.substr(oldPos, pos - oldPos));
        oldPos = pos+1;
        pos = str.find_first_of(c, oldPos);
    }
    
    result.push_back(str.substr(oldPos, string::npos));
    //UI_debug_pile("Exploded: \n");
    //print_list(result);
    //UI_debug_pile("End Explode\n");
    return result;
}


list<string> readIncludes(const list<string>& paths, const string& file)
{
    //UI_debug_pile("Reading %s\n", file.c_str());
    list<string> result;
    
    ifstream fin;
    fin.open(file.c_str());
    
    
    if(fin.fail())
    {
        fin.close();
        // Try other places
        bool gotIt = false;
        for(list<string>::const_iterator e = paths.begin(); e != paths.end(); e++)
        {
            fin.clear();
            //UI_debug_pile("Finding: %s\n", (*e + '/' + file).c_str());
            fin.open((*e + '/' + file).c_str());
            if(fin.fail())
            {
                //UI_debug_pile("Not found.\n");
                fin.close();
            }
            else
            {
                //UI_debug_pile("Found it!\n");
                gotIt = true;
                break;
            }
        }
        
        if(!gotIt)
            return result;
    }
    
    string path = getFilePath(file);
    
    
    string str;
    int numEmpties = 0;
    while(!fin.eof())
    {
        getline(fin, str);
        //UI_debug_pile("LINE: %s == %d\n", str.c_str(), (str == ""));
        if(str == "")  // For some reason, there's an infinite loop...
        {
            numEmpties++;
            if(numEmpties > 300)
                break;
            continue;
        }
        numEmpties = 0;
        
        if(str.find('#') != string::npos)
        {
            unsigned int pos = 0;
            while(isWhitespace(str[pos]))
            {
                pos++;
            }
            if(str[pos] == '#')
            {
                pos++;
                while(isWhitespace(str[pos]))
                {
                    pos++;
                }
                if(str.substr(pos, 7) == "include")
                {
                    //UI_debug_pile("Found an include\n-> %s\n", str.c_str());
                    removeUpTo(str, '#');
                    
                    removeQuantifiers(str);
                    
                    if(ioExists(path + str))
                        str = path + str;
                    else
                    {
                        //UI_debug_pile("Dependency %s not found locally...  Checking default paths.\n", str.c_str());
                        for(list<string>::const_iterator e = paths.begin(); e != paths.end(); e++)
                        {
                            //UI_debug_pile("Checking if %s exists... ", (*e + '/' + str).c_str());
                            if(ioExists(*e + '/' + str))
                            {
                                //UI_debug_pile("Yep\n");
                                str = *e + '/' + str;
                                break;
                            }
                            else
                            {
                                //UI_debug_pile("Nope\n");
                            }
                        }
                    }
                    
                    //UI_debug_pile("Pushing: %s\n", str.c_str());
                    result.push_back(str);
                }
            }
        }
    }
    
    fin.close();
    
    return result;
}

template<typename T>
int list_find(const list<T>& ls, T item)
{
    int i = 0;
    for(typename list<T>::const_iterator e = ls.begin(); e != ls.end(); e++)
    {
        if(*e == item)
            return i;
        i++;
    }
    return -1;
}


// Returns true on a new addition
bool addDepend(map<FileData*, list<FileData*> >& depends, map<string, FileData*>& fileDataHash, const string& parent, const string& include)
{
    //UI_debug_pile("Adding depend: %s, %s\n", parent.c_str(), include.c_str());
    FileData* fdp = fileDataHash[parent];
    FileData* fdi = fileDataHash[include];
    if(fdp == NULL)
    {
        fdp = new FileData(parent);
        fileDataHash[parent] = fdp;
    }
    if(fdi == NULL)
    {
        fdi = new FileData(include);
        fileDataHash[include] = fdi;
    }
    
    if(list_find(depends[fdp], fdi) < 0)
    {
        // Add to the dependencies.
        depends[fdp].push_back(fdi);
        return true;
    }
    return false;
}



void recurseIncludes(map<FileData*, list<FileData*> >& depends, map<string, FileData*>& fileDataHash, const list<string>& paths, const string& file, string path)
{
    list<string> includes = readIncludes(paths, file);
    for(list<string>::iterator e = includes.begin(); e != includes.end(); e++)
    {
        //UI_debug_pile("%s includes: %s\n", file.c_str(), e->c_str());
        if(addDepend(depends, fileDataHash, file, *e))
        {
            //UI_debug_pile("  file: %s, depend: %s, path: %s\n", file.c_str(), e->c_str(), path.c_str());
            recurseIncludes(depends, fileDataHash, paths, path + *e, path + getFilePath(*e));
            // Get the latest modification time.
            FileData* parent = fileDataHash[file];
            FileData* depend = fileDataHash[*e];
            if(parent != NULL && depend != NULL)
            {
                if(parent->getDependTime() < depend->getDependTime())
                    parent->setDependTime(depend->getDependTime());
            }
            /*else if(parent == NULL)
            {
                UI_debug_pile("NULL ptr: %s\n", file.c_str());
            }
            else if(depend == NULL)
            {
                UI_debug_pile("NULL ptr: %s\n", (*e).c_str());
            }*/
        }
    }
}



void printDepends(const list<string>& paths, const string& file)
{
    map<FileData*, list<FileData*> > depends;
    map<string, FileData*> fileDataHash;
    
    recurseIncludes(depends, fileDataHash, paths, file, "");
    
    for(map<FileData*, list<FileData*> >::iterator e = depends.begin(); e != depends.end(); e++)
    {
        for(list<FileData*>::iterator f = e->second.begin(); f != e->second.end(); f++)
            UI_print("Depend: %s depends on %s\n", e->first->getPath().c_str(), (*f)->getPath().c_str());
    }
}











string getFilePath(string file)
{
    unsigned int lastSlash = file.find_last_of('/');
    if(lastSlash != string::npos)
    {
        return file.substr(0, lastSlash+1);
    }
    else
        return "";
}

string getFileName(string file)
{
    unsigned int lastSlash = file.find_last_of('/');
    if(lastSlash != string::npos)
        file = file.substr(lastSlash, string::npos);
    return file;
}





bool mustRebuild(const string& objName, map<FileData*, list<FileData*> > depends, FileData* file)
{
    if(file == NULL)
    {
        UI_debug_pile(" Error: mustRebuild() passed a NULL file.\n");
        return true;
    }
    
    // If the object file is older than the source file, rebuild.
    //string obj = getBaseName(file->getPath()) + ".o";
    //removePath(obj);
    
    time_t tObj = ioTimeModified(objName);
    time_t tSrc = ioTimeModified(file->getPath());
    if(tObj <= tSrc)
        return true;
    
    
    //UI_debug_pile("Checking dependencies.\n");
    //UI_debug_pile("Depends size: %d\n", depends.size());
    // If any dependency is newer than the object file, then rebuild.
    time_t t = file->getDependTime();
    return (tObj <= t);
}



