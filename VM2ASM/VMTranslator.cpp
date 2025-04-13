#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;
struct stat s;
class Translator
{
    string operation, command, segment, offset, asm_line;
    int cond_count, func_count, num_lcl;
    bool is_init;

public:
    Translator(){}
    Translator(string pathname)
    {
        cond_count = 0, func_count = 0;
        is_init = false;

        if (is_file(pathname))
        {
            size_t lastindex = pathname.find_last_of(".");
            string rawname = pathname.substr(0, lastindex);
            ofstream outfile("./"+ rawname + ".asm");
            is_init = true;
            parse_vm_file(pathname, outfile);
            outfile.close();
        }
        else if (is_dir(pathname))
        {
            ofstream outfile("./"+ pathname + "/" + pathname + ".asm");
            vector<string> file_list = get_all_files(pathname);
            for (string file_name : file_list) 
            {
                parse_vm_file(pathname + '/' + file_name, outfile);
            }
            outfile.close();
        }
        else 
        {
            cout << pathname;
            cout << ": No such file or directory" << endl;
        }
    }

    void parse_vm_file(string filename, ofstream& outfile)
    {
        size_t firstindex = filename.find_first_of("/");
        size_t lastindex = filename.find_last_of(".");
        string name_wo_ext = filename.substr(0, lastindex);
        string true_name = name_wo_ext.substr(firstindex+1);

        if (filename.substr(lastindex + 1) != "vm")
        {
            cout << "file must be .vm file" << endl;
            return void();
        }
        
        ifstream vm_file(filename);

        if (vm_file.is_open())
        {
            if (!is_init)
            {
                outfile << "@SP\n@LCL\n@ARG\n@THIS\n@THAT\n";
                outfile << "@256\nD=A\n@SP\nM=D\n";
                string call_sys_init = "";
                call_func(func_count, 0, "Sys.init",  call_sys_init);
                outfile << call_sys_init;
                is_init = true;
            }
            
            for (string line; getline(vm_file, line);)
            {
                if (line.length() == 0)
                {
                    continue;
                }
                else
                {
                    if (line.substr(0, 2) == "//")
                    {
                        continue;
                    }
                    
                }

                asm_line = parse_command(true_name, line);
                outfile << asm_line;
            }
        }

        vm_file.close();
    }

    string parse_command(string frame_name, string line)
    {
        vector<string> commands;
        string output_assembly = "";
        string command = "";
        bool prev_space = true, is_space = true;
        bool prev_slash = false, is_slash = false;

        for (int i=0; i < line.length(); i++)
        {
            is_space = (line[i] == ' ' || line[i] == '\t');

            if (!is_space)
            {
                is_slash = (line[i] == '/');

                if (is_slash && prev_slash)
                {
                    break;
                }

                command += line[i];

                if (i == line.length() - 1)
                {
                    commands.push_back(command);
                }
            }
            else if (!prev_space)
            {
                commands.push_back(command);
                command = "";
            }
            
            prev_space = is_space;
            prev_slash = is_slash;
        }

        if (commands.size() == 1)
        {
            string cond = "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=M-D\nM=0\n@COND" + to_string(cond_count) + "\n0;JMP\n(TRUE" + to_string(cond_count) + ")\n@SP\nA=M-1\nM=1\nD=!D\n(COND" + to_string(cond_count) + ")\n@TRUE" + to_string(cond_count) + "\nD;";
            operation = commands[0];
            if (operation == "add")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=D+M\n";
            else if (operation == "sub")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=M-D\n";
            else if (operation == "neg")
                output_assembly += "@SP\nA=M\nA=A-1\nM=-M\n";
            else if (operation == "and")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=D&M\nM=D\n";
            else if (operation == "or")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=D|M\nM=D\n";
            else if (operation == "not")
                output_assembly += "@SP\nA=M\nA=A-1\nM=!M\n";
            else if (operation == "eq")
            {
                output_assembly += cond + "JEQ\n";
                cond_count++;
            }
            else if (operation == "gt")
            {
                output_assembly += cond + "JGT\n";
                cond_count++;
            }
            else if (operation == "lt")
            {
                output_assembly += cond + "JLT\n";
                cond_count++;
            }
            else if (operation == "return")
            {
                output_assembly += "@SP\nA=M-1\nD=M\n";
                output_assembly += "@ARG\nA=M\nM=D\nD=A\n";
                output_assembly += "@SP\nM=D+1\n";

                output_assembly += "@LCL\nD=M\n";
                output_assembly += "@5\nA=D-A\nD=M\n@SP\nA=M\nM=D\n";

                output_assembly += "@LCL\nD=M\n";
                output_assembly += "@1\nA=D-A\nD=M\n@THAT\nM=D\n";

                output_assembly += "@LCL\nD=M\n";
                output_assembly += "@2\nA=D-A\nD=M\n@THIS\nM=D\n";

                output_assembly += "@LCL\nD=M\n";
                output_assembly += "@3\nA=D-A\nD=M\n@ARG\nM=D\n";

                output_assembly += "@LCL\nD=M\n";
                output_assembly += "@4\nA=D-A\nD=M\n@LCL\nM=D\n";

                output_assembly += "@SP\nA=M\nA=M\n";
                output_assembly += "0;JMP\n";
            }
        }
        else if (commands.size() == 2)
        {
            command = commands[0];
            segment = commands[1];

            if (command == "label")
            {
                output_assembly += "(" + segment + ")\n";
            }
            else if (command == "if-goto")
            {
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\n";
                output_assembly += "@"+ segment + "\n";
                output_assembly += "D;JGT\n";
            }
            else if (command == "goto")
            {
                output_assembly += "@"+ segment + "\n";
                output_assembly += "0;JMP\n";
            }
        }

        else if (commands.size() == 3)
        {
            command = commands[0];
            segment = commands[1];
            offset = commands[2];

            if (segment == "local")
                output_assembly += "@LCL\nD=M\n";
            else if (segment == "argument")
                output_assembly += "@ARG\nD=M\n";
            else if (segment == "this")
                output_assembly += "@THIS\nD=M\n";
            else if (segment == "that")
                output_assembly += "@THAT\nD=M\n";
            else if (segment == "temp")
                output_assembly += "@5\nD=A\n";
            else if (segment == "pointer")
            {
                output_assembly += (offset == "0") ? "@THIS\nD=A\n" : "@THAT\nD=A\n";
                offset = "0";
            }
            else if (segment == "static")
            {
                output_assembly += "@" + frame_name + "." + offset + "\nD=A\n";
                offset = "0";
            }

            if (command == "push")
            {
                output_assembly += "@" + offset + "\n";
                if (segment != "constant")
                {
                    output_assembly += "A=D+A\nD=M\n";
                }
                else
                    output_assembly += "D=A\n";

                output_assembly += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }

            else if (command == "pop")
            {
                output_assembly += "@" + offset + "\n";
                if (segment != "constant")
                {
                    output_assembly += "D=D+A\n";
                    output_assembly += "@SP\nM=M-1\nA=M+1\nM=D\nA=A-1\nD=M\nA=A+1\nA=M\nM=D\n";
                }
            }

            else if (command == "function")
            {
                output_assembly += "(" + segment + ")\n";
                num_lcl = stoi(offset);
                for (int i=0; i < num_lcl; i++)
                {
                    output_assembly += "@0\nD=A\n";
                    output_assembly += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                }
            }

            else if (command == "call")
            {
                call_func(func_count, stoi(offset), segment,  output_assembly);
            }
        }

        return output_assembly;
    }

    void call_func(int& func_count, int offset, string func_name, string& output)
    {

        // Push 0 before calling the function
        // Serve as a placeholder for the return value of function that take no argument
        if (!offset && func_name != "Sys.init")
        {
            output += "@0\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; 
            offset = 1;
        }

        output += "@FUNC_" + to_string(func_count) + "\n";
        output += "D=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

        output += "@LCL\n";
        output += "D=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

        output += "@ARG\n";
        output += "D=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

        output += "@THIS\n";
        output += "D=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

        output += "@THAT\n";
        output += "D=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

        output += "@SP\nD=M\n";
        output += "@"+to_string(5+offset)+"\n";
        output += "D=D-A\n";
        output += "@ARG\nM=D\n";

        output += "@SP\nD=M\n";
        output += "@LCL\nM=D\n";
        
        output += "@"+ func_name + "\n";
        output += "0;JMP\n";

        output += "(FUNC_" + to_string(func_count++) + ")\n";
    }

    bool is_dir(const string& path) 
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) 
        {
            return false;
        }
        return (info.st_mode & S_IFDIR) != 0;
    }

    bool is_file(const string& path) {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) 
        {
            return false;
        }
        return (info.st_mode & S_IFREG) != 0;
    }

    vector<string> get_all_files(string path)
    {
        DIR *dir;
        struct dirent *ent;
        vector<string> all_files;
        string filename = "";
        if ((dir = opendir (path.c_str())) != NULL) 
        {
            /* print all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) 
            {
                filename = ent->d_name;
                if (filename != "." && filename != "..")
                    all_files.push_back(filename);
            }
            closedir(dir);
        } 
    
        return all_files;
    }
};

int main(int argc, char *argv[])
{
    string path_name = argv[1];
    Translator translate(path_name);
    return 0;
}
