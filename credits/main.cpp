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
void readNextLine(std::ifstream& fin, std::string& current_line, std::string& next_line, bool& extra_padding, int& line_count, bool& section_complete);
void printLangFile(std::ofstream& lang, const std::string& pack_namespace, const std::string& current_line, int line_count);
void addLineToJSON(json& j, json_node_t& current_control, const json& padding, const json& small_padding, const std::string& pack_namespace, bool extra_padding, const std::string& line_count);
void addContributorColors(std::string& current_line, int line_count, size_t name_position = 0);
void addSocialMediaColors(std::string& current_line, size_t colon_position);

//Allow push_back to work with the JSON node struct
void to_json(json& j, const json_node_t& node) {
    j = {{node.name, {{"$text",node.text}}}};
    if (!node.child.empty())
        j.push_back({"controls", node.child});
}

int main(int argc, char* argv[]) {

    if (argc != 3) { //Check for correct number of inputs
        std::cerr << "Usage: " << argv[0] << " <credits_input.txt> <pack_namespace>" << std::endl;
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
    std::ofstream lang("en_US_credits.lang");
    if (lang.fail()) {
        std::cerr << "Failed to create en_US_credits.lang." << std::endl;
        return -3;
    }
    std::cout << "Created en_US_credits.lang..." << std::endl;
    std::ofstream formatter("pack_credits_section.json");
    if (lang.fail()) {
        std::cerr << "Failed to create pack_credits_section.json." << std::endl;
        return -4;
    }
    std::cout << "Created pack_credits_section.json..." << std::endl;

    //Lang file
    std::string current_line, next_line;

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

    //Create credits stack_panel
    j[pack_namespace + "_credits"] = {
            {"type", "stack_panel"},
            {"size", { "100%", "100%c" }},
            {"controls", json::array()}
    };

    //Set up first line
    current_control.name = "credits_title@how_to_play_common.header";
    current_control.text = "tab." + pack_namespace + ".credits.section";
    j[pack_namespace + "_credits"]["controls"].push_back(current_control);
    extra_padding = true;
    readNextLine(fin,current_line,next_line,extra_padding,line_count,section_complete);

    //Read/write loop
    while (!(current_line == "============================") && !fin.eof() && line_count < 500) {

        //Separate each section into its own loop
        while (!section_complete && !fin.eof()) {

            //Print the line to both files
            printLangFile(lang,pack_namespace,current_line,line_count);
            addLineToJSON(j, current_control, padding, small_padding, pack_namespace, extra_padding,std::to_string(line_count));

            //Read next line
            extra_padding = false;
            readNextLine(fin,current_line,next_line,extra_padding,line_count,section_complete);

            if (line_count > 500) { //Safety measure - prevent massive files
                section_complete = true;
            }
        }
    }

    //Print final line and JSON
    if (fin.eof()) {
        printLangFile(lang,pack_namespace,current_line,line_count);
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
void readNextLine(std::ifstream& fin, std::string& current_line, std::string& next_line, bool& extra_padding, int& line_count, bool& section_complete) {
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

    //Add verbose contributor formatting
    size_t str_position = current_line.find("made by");
    if (str_position != std::string::npos) { //Only continue if the line has a verbose contributor credit
        addContributorColors(current_line,line_count,str_position + 8);
    }

    //Add single-line contributor formatting
    str_position = current_line.find(" - ");
    if (str_position != std::string::npos) { //Only continue if the line has a single-line credit
        addContributorColors(current_line,line_count);
    }

    //Add separate-line social media formatting
    str_position = current_line.find(':');
    if (str_position != std::string::npos) { //Only continue if the line may contain a unique social media credit
        addSocialMediaColors(current_line,str_position);
    }
}

//Prints one line of the language file
void printLangFile(std::ofstream& lang, const std::string& pack_namespace, const std::string& current_line, const int line_count) {

    //Create left side of line
    lang << "tab." << pack_namespace << ".credits." << line_count << "=";
    //Create right side of line
    lang << current_line << std::endl;

}

//Adds padding and line to JSON
void addLineToJSON(json& j, json_node_t& current_control, const json& padding, const json& small_padding, const std::string& pack_namespace, const bool extra_padding, const std::string& line_count) {

    //Print padding, unless on the first line
    if (extra_padding) {
        j[pack_namespace + "_credits"]["controls"].push_back(padding);
    }
    else {
        j[pack_namespace + "_credits"]["controls"].push_back(small_padding);
    }

    current_control.name = "credits_" + line_count + "@how_to_play_common.paragraph";
    current_control.text = "tab." + pack_namespace + ".credits." + line_count;

    j[pack_namespace + "_credits"]["controls"].push_back(current_control);
}

//Colors name and social media handle for contributors
//If name_position is not 0, the line is a verbose line
//name_position defaults to 0
void addContributorColors(std::string& current_line, const int line_count, const size_t name_position) {

    //Find important parts of line
    size_t str_position = current_line.find('(');
    size_t end_position = current_line.find(')');
    size_t divider_position = current_line.find(" - ");
    const std::string format_code = "§0";
    const size_t format_size = format_code.length();

    //Case 1: Non-verbose line with no social media credit
    //These lines are guaranteed to have only 1 contributor
    if (name_position == 0 && (str_position == std::string::npos || str_position > divider_position)) {

        current_line.insert(name_position, "§6"); //Gold before all contributor names
        current_line.insert(divider_position + format_size, "§r"); //Reset formatting before divider

    }

    //Case 2: Non-verbose line with a social media credit
    //These lines are guaranteed to have only 1 contributor
    else if (name_position == 0) {

        size_t pos_offset = 0;

        current_line.insert(name_position, "§6"); //Gold before all contributor names
        pos_offset += format_size;

        //Continue only if all expected formatting is in place
        if (str_position > end_position || end_position > divider_position) {
            std::cerr << "Error: Contributor credit on line " << line_count << " lacks proper formatting. Check manually!" << std::endl;
        }
        else {
            //Get social media substring
            const std::string social_media_handle = current_line.substr(str_position + 1 + pos_offset,end_position - str_position - 1);
            std::string social_media_color;

            //Add color based on platform
            if (social_media_handle.find("Twitter") != std::string::npos) { //Twitter/X
                social_media_color = "§r§3"; //Cyan
            }
            else if (social_media_handle.find("Discord") != std::string::npos) { //Discord
                social_media_color = "§r§5"; //Purple
            }
            else if (social_media_handle.find("Bluesky") != std::string::npos) { //Bluesky
                social_media_color = "§r§9"; //Blue (duh)
            }
            else if (social_media_handle.find("Fediverse") != std::string::npos) { //Fediverse (Mastodon and all other federated platforms)
                social_media_color = "§r§d"; //Light Purple
            }
            else if (social_media_handle.find("YouTube") != std::string::npos) { //YouTube
                social_media_color = "§r§c"; //Red
            }
            else { //Fallback for unknown social media
                social_media_color = "§r§a"; //Green
            }
            current_line.insert(str_position + 1 + pos_offset,social_media_color);
            pos_offset += format_size * 2;

            //End contact info formatting
            if (end_position != std::string::npos) {
                current_line.insert(end_position + pos_offset, "§r§6"); //Gold
                pos_offset += format_size * 2;
            }
            current_line.insert(divider_position + pos_offset, "§r"); //Reset formatting
        }
    }

    //Case 3: Verbose line with unknown number of contributors
    //It is expected that all verbose lines only use parentheses around social media handles
    //All verbose contributors have capitalized names so far
    else {

        //Iterate through string starting at first name
        std::vector<size_t> start_pos, social_media_pos, end_pos;

        //Initialize first contributor detected
        start_pos.push_back(name_position);
        bool awaiting_new_contributor = false, awaiting_social_media = true, awaiting_social_media_end = false;

        for (size_t pos = name_position + 1; pos < current_line.length(); pos++) {

            //First capitalized letter is counted as "start" of each contributor
            if (isupper(current_line.at(pos)) && awaiting_new_contributor && !awaiting_social_media_end) {
                start_pos.push_back(pos);
                awaiting_new_contributor = false;
                awaiting_social_media = true;
            }
            //Social media credits start with parentheses
            else if (current_line.at(pos) == '(' && !awaiting_social_media_end) {
                social_media_pos.push_back(pos + 1);
                awaiting_social_media = false;
                awaiting_social_media_end = true;
            }
            //Detect end of social media credits
            else if (current_line.at(pos) == ')' && awaiting_social_media_end) {
                end_pos.push_back(pos);
                awaiting_new_contributor = true;
                awaiting_social_media_end = false;
            }
            //Catch contributors without social media credits
            else if ((current_line.at(pos) == '.' || current_line.at(pos) == ':' || current_line.at(pos) == ',') && awaiting_social_media) {
                social_media_pos.push_back(std::string::npos);
                end_pos.push_back(pos);
                awaiting_new_contributor = true;
                awaiting_social_media = false;
            }
        }
        if (start_pos.size() != end_pos.size() || start_pos.size() != social_media_pos.size()) {
            std::cerr << "Error: Unknown formatting situation on line " << line_count << ". Check manually!" << std::endl;
        }
        else {
            size_t pos_offset = 0;
            std::string social_media_handle, social_media_color;

            for (size_t i = 0; i < start_pos.size(); i++) {
                current_line.insert(start_pos.at(i) + pos_offset, "§6"); //Gold before all contributor names
                pos_offset += format_size;
                if (social_media_pos.at(i) == std::string::npos) {
                    current_line.insert(end_pos.at(i) + pos_offset, "§r"); //Reset formatting
                    pos_offset += format_size;
                }
                else {
                    social_media_handle = current_line.substr(social_media_pos.at(i) + 1 + pos_offset,end_pos.at(i) - social_media_pos.at(i) - 1);
                    //Add color based on platform
                    if (social_media_handle.find("Twitter") != std::string::npos) { //Twitter/X
                        social_media_color = "§r§3"; //Cyan
                    }
                    else if (social_media_handle.find("Discord") != std::string::npos) { //Discord
                        social_media_color = "§r§5"; //Purple
                    }
                    else if (social_media_handle.find("Bluesky") != std::string::npos) { //Bluesky
                        social_media_color = "§r§9"; //Blue (duh)
                    }
                    else if (social_media_handle.find("Fediverse") != std::string::npos) { //Fediverse (Mastodon and all other federated platforms)
                        social_media_color = "§r§d"; //Light Purple
                    }
                    else if (social_media_handle.find("YouTube") != std::string::npos) { //YouTube
                        social_media_color = "§r§c"; //Red
                    }
                    else { //Fallback for unknown social media
                        social_media_color = "§r§a"; //Green
                    }
                    current_line.insert(social_media_pos.at(i) + pos_offset,social_media_color);
                    pos_offset += format_size * 2;
                    current_line.insert(end_pos.at(i) + pos_offset,"§r§6");
                    pos_offset += format_size * 2;
                    current_line.insert(end_pos.at(i) + 1 + pos_offset,"§r");
                    pos_offset += format_size;
                }
            }
        }
    }
}

//Colors separated-line social media handles
//These lines always start with the name of the social media, then a colon, then the handle after a space
void addSocialMediaColors(std::string& current_line, size_t colon_position) {

    std::string social_media_color = "§f"; //Default value not used by any social media
    if (current_line.find("Twitter") == 0) {
        social_media_color = "§3"; //Cyan
    }
    else if (current_line.find("Discord") == 0) {
        social_media_color = "§5"; //Purple
    }
    else if (current_line.find("Bluesky") == 0) {
        social_media_color = "§9"; //Blue (duh)
    }
    else if (current_line.find("Fediverse") == 0) {
        social_media_color = "§d"; //Light Purple
    }
    else if (current_line.find("YouTube") == 0) {
        social_media_color = "§c"; //Red
    }
    //Only insert color if a matching social media was found
    if (social_media_color != "§f") {
        current_line.insert(colon_position + 2, social_media_color);
    }
    //Otherwise, no error. Not all colon lines are for social media!
}