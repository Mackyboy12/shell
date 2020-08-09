#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <errno.h>

std::vector<std::string> Split(std::string input);
void Execute(const char *command, char *arglist[]);
bool BuiltInCom(const char *command, char *arglist[]);
char** conv(std::vector<std::string> source);
int main() {
  while (1) {
    // Takes input and splits it by space
    std::string input;
    std::cout << "$: ";
    getline(std::cin, input);
    if(input == "") continue;
    std::vector<std::string> parsed_string = Split(input);
    // Splits parsed_string into command and arglist
    const char * com = parsed_string.front().c_str();
    char ** arglist = conv(parsed_string);
    // Checks if it is a built in command and if not, execute it
    if(BuiltInCom(com, arglist) == 0){
        Execute(com, arglist);
    }
    delete[] arglist;
  }
}

std::vector<std::string> Split(std::string input) {
  std::vector<std::string> ret;
  std::istringstream f(input);
  std::string s;
  while (getline(f, s, ' ')) {
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

bool BuiltInCom(const char *command, char ** arglist){
  if(strcmp(command, "quit") == 0){
    delete[] arglist;
    exit(0);
  } else if(strcmp(command, "cd") == 0){
    std::cout << "Arglist: " << arglist[1] << std::endl;
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
  }
    return 0;
}

char** conv(std::vector<std::string> source){
  char ** dest = new char*[source.size() + 1];
  for(int i = 0; i < source.size(); i++) dest[i] = (char *)source.at(i).c_str();
  dest[source.size()] = NULL;
  return dest;
}
