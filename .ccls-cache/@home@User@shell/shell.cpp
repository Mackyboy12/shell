#include <cstring>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <errno.h>
#include <algorithm>
std::string USERDIR = getenv("HOME");
std::string ALIASFILE = USERDIR+"/shell/.alias";
std::vector<std::string> Split(std::string input, char delim);
void Execute(const char *command, char *arglist[]);
std::map<std::string, std::string> alias(std::string file);
bool BuiltInCom(const char *command, char *arglist[],int arglist_size);
char** conv(std::vector<std::string> source);
bool createAlias(std::string first, std::string sec);
std::string replaceAll(std::string data, std::map <std::string, std::string> dict);
int main() {
  while (1) {
    char path[100];
    getcwd(path, 100);
    char prompt[110] = "$[";
    strcat(prompt, path);
    strcat(prompt,"]: ");
    std::cout << prompt;
    // Takes input and splits it by space
    std::string input;
    getline(std::cin, input);
    if(input == "") continue;
    std::map<std::string, std::string> aliasDict = alias(ALIASFILE);
    input = replaceAll(input, aliasDict);
    std::vector<std::string> parsed_string = Split(input, ' ');
    // Splits parsed_string into command and arglist
    const char * com = parsed_string.front().c_str();
    char ** arglist = conv(parsed_string);
    // Checks if it is a built in command and if not, execute it
    if(BuiltInCom(com, arglist, parsed_string.size()) == 0){
        Execute(com, arglist);
    }
    delete[] arglist;
  }
}

std::vector<std::string> Split(std::string input, char delim) {
  std::vector<std::string> ret;
  std::istringstream f(input);
  std::string s;
  while (getline(f, s, delim)) {
    ret.push_back(s);
  }
  return ret;
}

void Execute(const char *command, char *arglist[]) {
  pid_t pid;
  //Creates a new proccess
  if ((pid = fork()) < 0) {
    std::cout << "Error: Cannot create new process" << std::endl;
    exit(-1);
  } else if (pid == 0) {
    //Executes the command
    if (execvp(command, arglist) < 0) {
      std::cout << "Could not execute command" << std::endl;
      exit(-1);
    } else {
      sleep(2);
    }
  }
  //Waits for command to finish
  if (waitpid(pid, NULL, 0) != pid) {
    std::cout << "Error: waitpid()";
    exit(-1);
  }
}

bool BuiltInCom(const char *command, char ** arglist, int arglist_size){
  if(strcmp(command, "quit") == 0){
    delete[] arglist;
    exit(0);
  } else if(strcmp(command, "cd") == 0){
    if(chdir(arglist[1]) < 0){
      switch(errno){
        case EACCES:
          std::cout << "Search permission denied." << std::endl;
          break;
        case EFAULT:
          std::cout << "Path points outside accesable adress space" << std::endl;
          break;
        case EIO:
          std::cout << "IO error" << std::endl;
          break;
        case ELOOP:
          std::cout << "Too many symbolic loops" << std::endl;
          break;
        case ENAMETOOLONG:
          std::cout << "Path is too long" << std::endl;
          break;
        case ENOENT:
          std::cout << "Path doesn't exist" << std::endl;
          break;
        case ENOTDIR:
          std::cout << "Path isn't a dir" << std::endl;
          break;

        default:
            std::cout << "Unknown error" << std::endl;
            break;
      }
      return 1;
    }
    return 1;
  } else if(strcmp(command, "alias") == 0){
    if(arglist_size < 2){
      std::cout << "[USAGE] Alias originalName:substituteName" << std::endl;
      return 1;
    }
    std::string strArg(arglist[1]);
    int numOfSpaces = std::count(strArg.begin(), strArg.end(), ':');
    if(numOfSpaces){
      std::vector<std::string> aliasPair = Split(strArg, ':');
      createAlias(aliasPair.at(0), aliasPair.at(1));
      return 1;
    } else {
      std::cout << "[USAGE] Alias originalName:substituteName" << std::endl;
      return 1;
    }
  }
  return 0;
}

char** conv(std::vector<std::string> source){
  char ** dest = new char*[source.size() + 1];
  for(int i = 0; i < source.size(); i++) dest[i] = (char *)source.at(i).c_str();
  dest[source.size()] = NULL;
  return dest;
}


std::map<std::string, std::string> alias(std::string file){
  std::map<std::string, std::string> aliasPair;
  std::string line;
  std::ifstream aliasFile;
  aliasFile.open(file);
  if(aliasFile.is_open()){
    while(getline(aliasFile, line)){
      auto pair = Split(line, ':');
      aliasPair.insert(std::make_pair(pair.at(0), pair.at(1)));
    }
  } else {
    std::cout << "Error: Cannot open alias file\n";
  }
  return aliasPair;
}
std::string replaceAll(std::string data, std::map <std::string, std::string> dict){
  for(std::pair <std::string, std::string> entry : dict){
      size_t start_pos = data.find(entry.first);
      while(start_pos != std::string::npos){
        data.replace(start_pos, entry.first.length(),entry.second);
        start_pos = data.find(entry.first, start_pos + entry.second.size());
      }

  }
  return data;
}
bool createAlias(std::string first, std::string second){
    std::ofstream aliasFile;
    aliasFile.open(ALIASFILE, std::ios_base::app);
    if(aliasFile.is_open()){
      aliasFile << first << ":"<< second << std::endl;
      return true;
    } else return false;

}
