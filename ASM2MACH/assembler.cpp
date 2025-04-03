#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <bitset>
#include <bits/stdc++.h>

using namespace std;

class Assembler
{
    string outLine = "";
    string dest = "";
    string variable_name = "";
    string line_no_space = "";
    bool destination = false, jump = false;
    map<string, string> comp_map;
    map<string, string> dest_map;
    map<string, string> jump_map;
    map<string, string> symbol_map;
    int var_index;
    int instruction_num;
    int address;

public:
    Assembler(string filename)
    {
        ifstream assembly_file(filename + ".asm");
        ofstream outfile("./output/" + filename + ".hack");
        if (assembly_file.is_open())
        {
            initialize_map();
            initialize_symbol_map();
            instruction_num = 0;

            for (string line; getline(assembly_file, line);)
            {
                if (line.length() == 0)
                {
                    continue;
                }
                else
                {
                    line_no_space = remove_space(line);
                    if (line_no_space.substr(0, 2) == "//")
                    {
                        continue;
                    }
                    parseLabel(remove_space(line_no_space), instruction_num);
                }
            }

            assembly_file.clear();            // clear fail and eof bits
            assembly_file.seekg(0, ios::beg); // back to the start!

            var_index = 16;
            for (string line; getline(assembly_file, line);)
            {
                if (line.length() == 0)
                {
                    continue;
                }
                else
                {
                    line_no_space = remove_space(line);
                    if (line_no_space.substr(0, 2) == "//")
                    {
                        continue;
                    }
                    parseVariable(remove_space(line_no_space));
                }
            }

            assembly_file.clear();            // clear fail and eof bits
            assembly_file.seekg(0, ios::beg); // back to the start

            for (string line; getline(assembly_file, line);)
            {
                if (line.length() == 0)
                {
                    continue;
                }
                else
                {
                    line_no_space = remove_space(line);
                    if (line_no_space.substr(0, 2) == "//" || line_no_space[0] == '(')
                    {
                        continue;
                    }
                    outLine = translate(line_no_space);
                    outfile << outLine;
                }
            }
            assembly_file.close();
            outfile.close();
        }
    }

    void print_symbol()
    {
        for (const auto &elem : symbol_map)
        {
            cout << elem.first << " " << elem.second << "\n";
        }
    }

    void initialize_map()
    {
        comp_map["0"] = "0101010";
        comp_map["1"] = "0111111";
        comp_map["-1"] = "0111010";
        comp_map["D"] = "0001100";
        comp_map["A"] = "0110000";
        comp_map["M"] = "1110000";
        comp_map["!D"] = "0001101";
        comp_map["!A"] = "0110001";
        comp_map["!M"] = "1110001";
        comp_map["-D"] = "0001111";
        comp_map["-A"] = "0110011";
        comp_map["-M"] = "1110011";
        comp_map["D+1"] = "0011111";
        comp_map["A+1"] = "0110111";
        comp_map["M+1"] = "1110111";
        comp_map["D-1"] = "0001110";
        comp_map["A-1"] = "0110010";
        comp_map["M-1"] = "1110010";
        comp_map["D+A"] = "0000010";
        comp_map["D+M"] = "1000010";
        comp_map["D-A"] = "0010011";
        comp_map["D-M"] = "1010011";
        comp_map["A-D"] = "0000111";
        comp_map["M-D"] = "1000111";
        comp_map["D&A"] = "0000000";
        comp_map["D&M"] = "1000000";
        comp_map["D|A"] = "0010101";
        comp_map["D|M"] = "1010101";

        dest_map["M"] = "001";
        dest_map["D"] = "010";
        dest_map["DM"] = "011";
        dest_map["MD"] = "011";
        dest_map["A"] = "100";
        dest_map["AM"] = "101";
        dest_map["MA"] = "101";
        dest_map["AD"] = "110";
        dest_map["DA"] = "110";
        dest_map["ADM"] = "111";
        dest_map["null"] = "000";

        jump_map["JGT"] = "001";
        jump_map["JEQ"] = "010";
        jump_map["JGE"] = "011";
        jump_map["JLT"] = "100";
        jump_map["JNE"] = "101";
        jump_map["JLE"] = "110";
        jump_map["JMP"] = "111";
        jump_map["null"] = "000";
    }

    void initialize_symbol_map()
    {
        symbol_map["R0"] = "0";
        symbol_map["R1"] = "1";
        symbol_map["R2"] = "2";
        symbol_map["R3"] = "3";
        symbol_map["R4"] = "4";
        symbol_map["R5"] = "5";
        symbol_map["R6"] = "6";
        symbol_map["R7"] = "7";
        symbol_map["R8"] = "8";
        symbol_map["R9"] = "9";
        symbol_map["R10"] = "10";
        symbol_map["R11"] = "11";
        symbol_map["R12"] = "12";
        symbol_map["R13"] = "13";
        symbol_map["R14"] = "14";
        symbol_map["R15"] = "15";
        symbol_map["SP"] = "0";
        symbol_map["LCL"] = "1";
        symbol_map["ARG"] = "2";
        symbol_map["THIS"] = "3";
        symbol_map["THAT"] = "4";
        symbol_map["SCREEN"] = "16384";
        symbol_map["KBD"] = "24576";
    }

    string remove_space(string line)
    {
        string tmp_line;
        for (const char &letter : line)
        {
            if (letter != ' ')
            {
                tmp_line += letter;
            }
        }
        return tmp_line;
    }

    bool check_address_num(string address)
    {
        for (char letter : address)
        {
            if (letter < '0' || letter > '9')
            {
                return false;
            }
        }
        return true;
    }

    void parseLabel(string line, int &instruction_num)
    {
        if (line[0] == '(')
        {
            variable_name = "";

            for (char letter : line.substr(1))
            {
                if (letter == ')')
                {
                    break;
                }
                else
                {
                    variable_name += letter;
                }
            }
            symbol_map[variable_name] = to_string(instruction_num);
            return;
        }

        instruction_num++;
    }

    void parseVariable(string line)
    {
        if (line[0] == '@')
        {
            if (!check_address_num(line.substr(1)))
            {
                variable_name = line.substr(1);
                if (symbol_map.find(variable_name) == symbol_map.end())
                {
                    symbol_map[variable_name] = to_string(var_index++);
                }
            }
        }
    }

    string translate(string assembly_line)
    {

        if (assembly_line[0] == '@')
        {

            if (check_address_num(assembly_line.substr(1)))
            {
                address = stoi(assembly_line.substr(1));
            }
            else
            {
                address = stoi(symbol_map[assembly_line.substr(1)]);
            }

            return "0" + bitset<15>(address).to_string() + "\n";
        }

        else
        {
            jump = false, destination = false;
            string str_dest = "", str_comp = "", str_jump = "";
            vector<string> instruction_parts;
            instruction_parts.push_back("");
            for (char letter : assembly_line)
            {
                if (letter != '=' && letter != ';')
                {
                    if (letter != ' ')
                    {
                        instruction_parts.back() += letter;
                    }
                    else
                    {
                        instruction_parts.back() += "";
                    }
                }
                else if (letter == '=')
                {
                    destination = true;
                    instruction_parts.push_back("");
                }
                else if (letter == ';')
                {

                    jump = true;
                    instruction_parts.push_back("");
                }
            }

            if (instruction_parts.size() == 3)
            {
                cout << "Yeah";
            }

            if (destination && jump)
            {
                str_dest = instruction_parts[0];
                str_comp = instruction_parts[1];
                str_jump = instruction_parts[2];
            }
            else if (destination)
            {
                str_dest = instruction_parts[0];
                str_comp = instruction_parts[1];
                str_jump = "null";
            }
            else if (jump)
            {
                str_dest = "null";
                str_comp = instruction_parts[0];
                str_jump = instruction_parts[1];
            }

            string comp_code = comp_map[str_comp];
            string dest_code = dest_map[str_dest];
            string jump_code = jump_map[str_jump];

            return "111" + comp_code + dest_code + jump_code + "\n";
        }
    }
};

int main(int argc, char *argv[])
{
    string file_name = argv[1];
    size_t lastindex = file_name.find_last_of(".");
    string rawname = file_name.substr(0, lastindex);
    Assembler the_file(rawname);
    return 0;
}
