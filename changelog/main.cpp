#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

//Used to easily modify JSON before adding to the main object
struct json_node_t {
    std::string name;
    std::string text;
    std::vector<json_node_t> child;
};

std::string namespaceLine(std::string line);
void readNextLine(std::ifstream& fin, std::string& current_line, std::string& next_line, std::string& extra_formatting, bool& extra_padding, int& line_count, bool& section_complete);
void printLangFile(std::ofstream& lang, const std::string& pack_namespace, const std::string& section_title, const std::string& current_line, const std::string& extra_formatting, int line_count);
void addLineToJSON(json& j, json_node_t& current_control, const json& padding, const json& small_padding, const std::string& pack_namespace, const std::string& section_title, bool extra_padding, const std::string& line_count);

//Allow push_back to work with the JSON node struct
void to_json(json& j, const json_node_t& node) {
    j = {{node.name, {{"$text",node.text}}}};
    if (!node.child.empty())
        j.push_back({"controls", node.child});
}

int main(int argc, char* argv[]) {

    if (argc != 3) { //Check for correct number of inputs
        std::cerr << "Usage: " << argv[0] << " <changelog_input.txt> <pack_namespace>" << std::endl;
        return -1;
    }

    //Open input file
    std::ifstream fin(argv[1]);
    if (fin.fail()) {
        std::cerr << "Failed to open " << argv[1] << std::endl;
        return -2;
    }
    std::cout << "Opened " << argv[1] << "..." << std::endl;

    //Declare pack namespace
    std::string pack_namespace = namespaceLine(argv[2]);

    //Open output files
    std::ofstream lang("en_US_changelog.lang");
    if (lang.fail()) {
        std::cerr << "Failed to create en_US_changelog.lang." << std::endl;
        return -3;
    }
    std::cout << "Created en_US_changelog.lang..." << std::endl;
    std::ofstream formatter("pack_changelog_section.json");
    if (lang.fail()) {
        std::cerr << "Failed to create pack_changelog_section.json." << std::endl;
        return -4;
    }
    std::cout << "Created pack_changelog_section.json..." << std::endl;

    //Lang file
    std::string current_line, next_line, extra_formatting;

    //JSON file
    json j;
    json_node_t json_section_title, current_control;

    //Padding objects used to separate lines
    json padding = json::parse(R"(
    {
       "padding@how_to_play_common.padding": {}
    }
    )");
    json small_padding = json::parse(R"(
    {
       "small_padding@how_to_play_common.small_padding": {}
    }
    )");

    //General line behavior vars
    bool section_complete = false, extra_padding = false;
    int line_count = 0;
    std::string section_title;

    //Print title
    readNextLine(fin,current_line,next_line,extra_formatting,extra_padding,line_count,section_complete);
    lang << "tab." << pack_namespace << ".changelog.title=";
    lang << extra_formatting << current_line << std::endl;

    //Print release date
    readNextLine(fin,current_line,next_line,extra_formatting,extra_padding,line_count,section_complete);
    extra_formatting = "§7";
    lang << "tab." << pack_namespace << ".changelog.date=";
    lang << extra_formatting << current_line << std::endl;

    //Print rules
    readNextLine(fin,current_line,next_line,extra_formatting,extra_padding,line_count,section_complete);
    extra_formatting = "";
    lang << "tab." << pack_namespace << ".changelog.rules=";
    lang << extra_formatting << current_line << std::endl;

    //Set up first section
    readNextLine(fin,current_line,next_line,extra_formatting,extra_padding,line_count,section_complete);

    //Read/write loop
    while (!(current_line == "============================") && !fin.eof() && line_count < 500) {

        //Separate each section into its own loop
        while (!section_complete && !fin.eof()) {

            //Print the line to both files
            printLangFile(lang,pack_namespace,section_title,current_line,extra_formatting,line_count);
            addLineToJSON(j, current_control, padding, small_padding, pack_namespace, section_title, extra_padding,std::to_string(line_count));

            //Read next line
            extra_padding = false;
            readNextLine(fin,current_line,next_line,extra_formatting,extra_padding,line_count,section_complete);

            if (line_count > 500) { //Safety measure - prevent massive files
                section_complete = true;
            }

        }

        //Iterate section
        if (!fin.eof()) { //Prevent final line from changing section

            //Update section
            section_title = namespaceLine(current_line);
            section_complete = false;

            //Reset line count
            line_count = 1;

            //Separate lang entries
            lang << std::endl;

            //Add new section object to JSON
            j["changelog_" + section_title + "_section"] = {
                    {"type", "stack_panel"},
                    {"size", { "100%", "100%c" }},
                    {"controls", json::array()}
            };
        }

    }

    //Print final line and JSON
    if (fin.eof()) {
        printLangFile(lang,pack_namespace,section_title,current_line,extra_formatting,line_count);
        addLineToJSON(j, current_control, padding, small_padding, pack_namespace, extra_padding,std::to_string(line_count));

        formatter << j.dump(2) << std::endl;
    }

    //Close files
    fin.close();
    lang.close();
    formatter.close();

    std::cout << "Finished!" << std::endl;

    return 0;
}

//Formats text into nice, namespaced format
std::string namespaceLine(std::string line) {
    while ((!isalnum(line.front())) && line.size() > 1) { //Start with alphanumeric
        line.erase(0, 1);
    }
    while ((!isalnum(line.back())) && line.size() > 1) { //End with alphanumeric
        line.pop_back();
    }
    for (char& i : line) {
        i = tolower(i); //Lowercase everything
        if (i == ' ') {
            i = '_'; //Underscores instead of spaces
        }
    }
    return line;
}

//Reads a single line and configures formatting variables
//All vars passed by reference
void readNextLine(std::ifstream& fin, std::string& current_line, std::string& next_line, std::string& extra_formatting, bool& extra_padding, int& line_count, bool& section_complete) {
    do {
        if (current_line.empty()) {
            extra_padding = true; //Mark extra padding where there are lines with two newlines
        }
        std::getline(fin, next_line);
        if (next_line.front() == '=') {
            section_complete = true;
        }
        current_line = next_line;
    } while (current_line.empty()); //Skip empty lines
    line_count++; //Increment line count

    //Determine extra formatting
    if (current_line.front() != '-' && current_line.front() != ' ' && current_line.front() != '=') { //For feature categories
        extra_formatting = "§l"; //Bold
    }
    else if (current_line.front() == '=') { //For section titles
        extra_formatting = "§l§o"; //Bold and italics
    }
    else if (current_line.front() == ' ') { //For indented lines
        extra_formatting = "§7"; //Gray
    }
    else { //For everything else
        extra_formatting = ""; //Default
    }

    //Add contributor formatting
    size_t str_position = current_line.find("(Thanks");
    if (str_position != std::string::npos) { //Only continue if the line has a contributor credit
        current_line.insert(str_position + 1, "§6"); //Gold
        str_position = current_line.find("!)");
        if (str_position != std::string::npos) {
            current_line.insert(str_position + 1, "§r"); //Clear formatting
        }
        else {
            std::cerr << "Error: Contributor credit found on line " << line_count << " without the proper closing parentheses. Check manually!" << std::endl;
        }
    }
}

//Prints one line of the language file
void printLangFile(std::ofstream& lang, const std::string& pack_namespace, const std::string& section_title, const std::string& current_line, const std::string& extra_formatting, const int line_count) {

    //Create left side of line
    lang << "tab." << pack_namespace << ".changelog." << section_title << "." << line_count << "=";
    //Create right side of line
    lang << extra_formatting << current_line << std::endl;

}

//Adds padding and line to JSON
void addLineToJSON(json& j, json_node_t& current_control, const json& padding, const json& small_padding, const std::string& pack_namespace, const std::string& section_title, const bool extra_padding, const std::string& line_count) {

    //Print padding, unless on the first line
    if (extra_padding && line_count != "1") {
        j["changelog_" + section_title + "_section"]["controls"].push_back(padding);
    }
    else if (line_count != "1") {
        j["changelog_" + section_title + "_section"]["controls"].push_back(small_padding);
    }

    current_control.name = section_title + "_" + line_count + "@how_to_play_common.paragraph";
    current_control.text = "tab." + pack_namespace + ".changelog." + section_title + '.' + line_count;

    j["changelog_" + section_title + "_section"]["controls"].push_back(current_control);
}
