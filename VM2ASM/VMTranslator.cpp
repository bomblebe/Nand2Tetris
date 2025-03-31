#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Translator
{
    string operation, command, segment, offset, asm_line;
    int cond_count;

public:
    Translator(string filename)
    {
        ifstream vm_file(filename + ".vm");
        ofstream outfile("./output/"+ filename + ".asm");
        if (vm_file.is_open())
        {

            outfile << "@SP\n@LCL\n@ARG\n@THIS\n@THAT\n";
            cond_count = 0;

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

                asm_line = parse_command(line);
                outfile << asm_line;
            }

            vm_file.close();
            outfile.close();
        }
    }

    string parse_command(string line)
    {
        vector<string> commands;
        string output_assembly = "";
        commands.push_back("");
        for (char letter : line)
        {
            if (letter != ' ')
            {
                commands.back() = commands.back() + letter;
            }
            else
            {
                commands.push_back("");
            }
        }

        if (commands.size() == 1)
        {
            string cond = "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=M-D\nM=0\n@COND" + to_string(cond_count) + "\n0;JMP\n(TRUE" + to_string(cond_count) + ")\n@SP\nA=M-1\nM=-1\nD=!D\n(COND" + to_string(cond_count) + ")\n@TRUE" + to_string(cond_count) + "\nD;";
            operation = commands[0];
            if (operation == "add")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=D+M\nM=D\n";
            else if (operation == "sub")
                output_assembly += "@SP\nM=M-1\nA=M\nD=M\nA=A-1\nD=M-D\nM=D\n";
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
                output_assembly += "@Foo." + offset + "\nD=A\n";
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
        }

        return output_assembly;
    }
};

int main(int argc, char *argv[])
{
    string file_name = argv[1];
    size_t lastindex = file_name.find_last_of(".");
    string rawname = file_name.substr(0, lastindex);
    Translator the_file(rawname);
    return 0;
}
