//This is the data processing C++ program for the Capstone project
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "json.hpp" // Include nlohmann/json library
#include <numeric>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <functional>

std:: string readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if(!file.is_open()){
        throw std::runtime_error("Could not open the file: " +fileName);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void writeFile(const std::string& fileName, const std::string& content){
    //This is to write the content into the file, while handling potential errors.
    std::ofstream file(fileName);
    if(!file.is_open()){
        throw std::runtime_error("Could not open the file for writing: " +fileName);
    }
    file << content;
    file.close();

}
struct DataRecord{
    std::string id;int age;double value;std::string name;std::string category;
    DataRecord(const std::string& i, int a, double v, const std::string& n, const std::string& c)
        : id(i), age(a), value(v), name(n), category(c) {}
    DataRecord() : id(""), age(0), value(0.0), name(""), category("") {}
};
std::vector<DataRecord> parseData(const std::string& fileName){
    //This is to parse a json file and extract data into a vector of DataRecord structs.
    std::vector<DataRecord> records;

    // Read raw file content
    std::string fileContent = readFile(fileName);

    // Parse JSON safely
    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(fileContent);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Invalid JSON format in file: " + fileName);
    }

    // Ensure JSON root is an array
    if (!jsonData.is_array()) {
        throw std::runtime_error("JSON root must be an array");
    }

    // Parse each JSON object
    for (const auto& item : jsonData) {

        // Ensure item is an object
        if (!item.is_object()) {
            throw std::runtime_error("Invalid record: expected JSON object");
        }

        DataRecord record;

        // Required field checks + type validation
        if (!item.contains("id")||!item["id"].is_string())
            throw std::runtime_error("Invalid 'id' field");

        if (!item.contains("age") || !item["age"].is_number_integer())
            throw std::runtime_error("Missing or invalid 'age' field");

        if (!item.contains("value") || !item["value"].is_number())
            throw std::runtime_error("Missing or invalid 'value' field");

        if (!item.contains("name") || !item["name"].is_string())
            throw std::runtime_error("Missing or invalid 'name' field");

        if (!item.contains("category") || !item["category"].is_string())
            throw std::runtime_error("Missing or invalid 'category' field");

        // Assign values
        record.id = item["id"];
        record.age = item["age"];
        record.value = item["value"];
        record.name = item["name"];
        record.category = item["category"];

        // Logical validation
        if (record.age < 0)
            throw std::runtime_error("Invalid age: must be non-negative");

        if (record.name.empty())
            throw std::runtime_error("Name cannot be empty");

        records.push_back(record);
    }

    return records;
}
void calculateStatistics(const std::vector<DataRecord>& records){
    //This is to calculate the mean,mode, median and standard deviation of the 'value' field in the records.
    if(records.empty()){
        std::cout<<"No records to process."<<std::endl;
        return;
    }
    double sum=0;
    
    for(int i=0;i<records.size();i++){
        sum+=records[i].value;
    }
    double mean=sum/records.size();
    std::cout<<"Statistical Analysis Results"<<std::endl;
    std::cout<<"============================"<<std::endl;
    std::cout<<"Total Records Analyzed: "<<records.size()<<std::endl;
    std::cout<<"Mean: "<<mean<<std::endl;
    //Mode calculation
    std::vector<double> values;
    for(const auto& record:records){
        values.push_back(record.value);
    }
    std::sort(values.begin(),values.end());
    double mode=values[0];
    int maxCount=1;
    for(int i=0;i<values.size();i++){
        int count=1;
        for(int j=i+1;j<values.size();j++){
            if(values[j]==values[i]){
                count++;
            }
            else{
                break;
            }
        }
        if(count>maxCount){
            maxCount=count;
            mode=values[i];
        }
    }
    std::cout<<"Mode: "<<mode<<std::endl;
    //Median calculation
    double median=0;
    if(values.size()%2==0){
        int midIndex1=values.size()/2;
        int midIndex2=midIndex1-1;
        median=(values[midIndex1]+values[midIndex2])/2;
    }
    else{
        int midIndex=values.size()/2;
        median=values[midIndex];
    }
    std::cout<<"Median: "<<median<<std::endl;
    std::cout<<"Maximum Count of the mode: "<<maxCount<<std::endl;
    double range=values[values.size()-1]-values[0];
    std::cout<<"Range: "<<range<<std::endl;
    //Standard Deviation calculation
    double varianceSum=0;
    for(const auto& record:records){
        varianceSum+=(record.value-mean)*(record.value-mean);
    }
    double standardDeviation=sqrt(varianceSum/records.size());
    std::cout<<"Standard Deviation: "<<standardDeviation<<std::endl;
    std::cout<<"============================"<<std::endl;
}
std::vector<DataRecord> filterRecordsByPredicate(const std::vector<DataRecord>& records, const std::function<bool(const DataRecord&)>& predicate){
    //This is to filter records based on a predicate.
    std::vector<DataRecord> filteredRecords;
    for(const auto& record:records){
        if(predicate(record)){
            filteredRecords.push_back(record);
        }
    }
    return filteredRecords;
}
void normalizeCategoryNames(std::vector<DataRecord>& records)
{
    //This is to normalize the category names to uppercase.
    for (auto& r : records) {
        std::transform(r.category.begin(), r.category.end(),
                       r.category.begin(), ::toupper);
    }
}
//Aggregation function example: Calculate total value per category
std::unordered_map<std::string, double> aggregateValuesByCategory(const std::vector<DataRecord>& records) {
    std::unordered_map<std::string, double> categoryTotals;
    for (const auto& record : records) {
        categoryTotals[record.category] += record.value;
    }
    return categoryTotals;
}

void testCases(){
    std::cout << "\n========== TEST SUITE START ==========" << std::endl;
    
    try {
        // TEST 1: Normal Data - Create and parse valid JSON
        std::cout << "\n[TEST 1] Normal Data Parsing" << std::endl;
        nlohmann::json normalData = nlohmann::json::array();
        normalData.push_back({{"id", "1"}, {"age", 25}, {"value", 100.5}, {"name", "Alice"}, {"category", "TypeA"}});
        normalData.push_back({{"id", "2"}, {"age", 30}, {"value", 200.75}, {"name", "Bob"}, {"category", "TypeB"}});
        normalData.push_back({{"id", "3"}, {"age", 35}, {"value", 150.25}, {"name", "Charlie"}, {"category", "TypeA"}});
        writeFile("normal_data.json", normalData.dump());
        auto normalRecords = parseData("normal_data.json");
        std::cout << "✓ Parsed " << normalRecords.size() << " records successfully" << std::endl;
        calculateStatistics(normalRecords);
        
        // TEST 2: Boundary Case - Single Record
        std::cout << "\n[TEST 2] Boundary Case - Single Record" << std::endl;
        nlohmann::json singleRecord = nlohmann::json::array();
        singleRecord.push_back({{"id", "1"}, {"age", 50}, {"value", 999.99}, {"name", "Single"}, {"category", "Unique"}});
        writeFile("single_record.json", singleRecord.dump());
        auto singleRecords = parseData("single_record.json");
        calculateStatistics(singleRecords);
        
        // TEST 3: Boundary Case - Zero Values
        std::cout << "\n[TEST 3] Boundary Case - Zero Values" << std::endl;
        nlohmann::json zeroData = nlohmann::json::array();
        zeroData.push_back({{"id", "1"}, {"age", 0}, {"value", 0.0}, {"name", "Zero"}, {"category", "Zero"}});
        zeroData.push_back({{"id", "2"}, {"age", 1}, {"value", 0.0}, {"name", "AlsoZero"}, {"category", "Zero"}});
        writeFile("zero_data.json", zeroData.dump());
        auto zeroRecords = parseData("zero_data.json");
        calculateStatistics(zeroRecords);
        
        // TEST 4: Edge Case - Large Values
        std::cout << "\n[TEST 4] Edge Case - Large Values" << std::endl;
        nlohmann::json largeData = nlohmann::json::array();
        largeData.push_back({{"id", "1"}, {"age", 100}, {"value", 999999.99}, {"name", "Large"}, {"category", "Big"}});
        largeData.push_back({{"id", "2"}, {"age", 120}, {"value", 888888.88}, {"name", "Larger"}, {"category", "Big"}});
        writeFile("large_data.json", largeData.dump());
        auto largeRecords = parseData("large_data.json");
        calculateStatistics(largeRecords);
        
        // TEST 5: Edge Case - Duplicate Values (Mode Testing)
        std::cout << "\n[TEST 5] Edge Case - Duplicate Values (Mode Testing)" << std::endl;
        nlohmann::json duplicateData = nlohmann::json::array();
        duplicateData.push_back({{"id", "1"}, {"age", 25}, {"value", 50.0}, {"name", "D1"}, {"category", "Dup"}});
        duplicateData.push_back({{"id", "2"}, {"age", 26}, {"value", 50.0}, {"name", "D2"}, {"category", "Dup"}});
        duplicateData.push_back({{"id", "3"}, {"age", 27}, {"value", 50.0}, {"name", "D3"}, {"category", "Dup"}});
        duplicateData.push_back({{"id", "4"}, {"age", 28}, {"value", 75.0}, {"name", "D4"}, {"category", "Dup"}});
        writeFile("duplicate_data.json", duplicateData.dump());
        auto duplicateRecords = parseData("duplicate_data.json");
        calculateStatistics(duplicateRecords);
        
        // TEST 6: Filter by Age
        std::cout << "\n[TEST 6] Filter Records by Age (>30)" << std::endl;
        auto filtered = filterRecordsByPredicate(normalRecords, [](const DataRecord& r){ return r.age > 30; });
        std::cout << "✓ Filtered " << filtered.size() << " records with age > 30" << std::endl;
        for(const auto& r : filtered) {
            std::cout << "  - " << r.name << ", Age: " << r.age << std::endl;
        }
        
        // TEST 7: Filter by Value
        std::cout << "\n[TEST 7] Filter Records by Value (>150)" << std::endl;
        auto valueFiltered = filterRecordsByPredicate(normalRecords, [](const DataRecord& r){ return r.value > 150; });
        std::cout << "✓ Filtered " << valueFiltered.size() << " records with value > 150" << std::endl;
        
        // TEST 8: Normalize Categories
        std::cout << "\n[TEST 8] Normalize Category Names to Uppercase" << std::endl;
        normalizeCategoryNames(normalRecords);
        std::cout << "✓ Categories normalized:" << std::endl;
        for(const auto& r : normalRecords) {
            std::cout << "  - " << r.name << ": " << r.category << std::endl;
        }
        
        // TEST 9: Aggregate by Category
        std::cout << "\n[TEST 9] Aggregate Values by Category" << std::endl;
        auto aggregated = aggregateValuesByCategory(normalRecords);
        std::cout << "✓ Category Totals:" << std::endl;
        for(const auto& pair : aggregated) {
            std::cout << "  - " << pair.first << ": " << pair.second << std::endl;
        }
        
        // TEST 10: Error Handling - Invalid JSON
        std::cout << "\n[TEST 10] Error Handling - Invalid JSON Format" << std::endl;
        writeFile("invalid_json.json", "{invalid json}");
        try {
            parseData("invalid_json.json");
            std::cout << "✗ Should have thrown exception" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "✓ Caught expected error: " << e.what() << std::endl;
        }
        
        // TEST 11: Error Handling - Negative Age
        std::cout << "\n[TEST 11] Error Handling - Negative Age" << std::endl;
        nlohmann::json negativeAge = nlohmann::json::array();
        negativeAge.push_back({{"id", "1"}, {"age", -5}, {"value", 100.0}, {"name", "BadAge"}, {"category", "Bad"}});
        writeFile("negative_age.json", negativeAge.dump());
        try {
            parseData("negative_age.json");
            std::cout << "✗ Should have thrown exception" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "✓ Caught expected error: " << e.what() << std::endl;
        }
        
        // TEST 12: Error Handling - Empty Name
        std::cout << "\n[TEST 12] Error Handling - Empty Name" << std::endl;
        nlohmann::json emptyName = nlohmann::json::array();
        emptyName.push_back({{"id", "1"}, {"age", 25}, {"value", 100.0}, {"name", ""}, {"category", "Empty"}});
        writeFile("empty_name.json", emptyName.dump());
        try {
            parseData("empty_name.json");
            std::cout << "✗ Should have thrown exception" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "✓ Caught expected error: " << e.what() << std::endl;
        }
        
        // TEST 13: Error Handling - Missing Field
        std::cout << "\n[TEST 13] Error Handling - Missing Field" << std::endl;
        nlohmann::json missingField = nlohmann::json::array();
        missingField.push_back({{"id", "1"}, {"age", 25}, {"value", 100.0}, {"category", "Incomplete"}});
        writeFile("missing_field.json", missingField.dump());
        try {
            parseData("missing_field.json");
            std::cout << "✗ Should have thrown exception" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "✓ Caught expected error: " << e.what() << std::endl;
        }
        
        // TEST 14: File I/O - Write and Read
        std::cout << "\n[TEST 14] File I/O - Write and Read" << std::endl;
        std::string testContent = "Test content for file I/O";
        writeFile("test_io.txt", testContent);
        std::string readContent = readFile("test_io.txt");
        if(readContent == testContent) {
            std::cout << "✓ File written and read successfully" << std::endl;
        } else {
            std::cout << "✗ Content mismatch" << std::endl;
        }
        
        // TEST 15: Empty Dataset
        std::cout << "\n[TEST 15] Empty Dataset" << std::endl;
        nlohmann::json emptyArray = nlohmann::json::array();
        writeFile("empty_data.json", emptyArray.dump());
        auto emptyRecords = parseData("empty_data.json");
        calculateStatistics(emptyRecords);
        
    } catch (const std::exception& e) {
        std::cout << "✗ Unexpected error: " << e.what() << std::endl;
    }
    
    std::cout << "\n========== TEST SUITE END ==========" << std::endl;
}
//After testing, the code is entirely valid.
int main(){
    //testCases();
    std::vector<DataRecord> records;
    bool dataLoaded = false;

    while (true) {
        std::cout << "\n==== Data Processing Application ====\n";
        std::cout << "1. Load JSON data file\n";
        std::cout << "2. Display statistical analysis\n";
        std::cout << "3. Filter records\n";
        std::cout << "4. Normalize category names\n";
        std::cout << "5. Aggregate values by category\n";
        std::cout << "6. Exit\n";
        std::cout << "Choose an option: ";

        int choice;
        std::cin >> choice;

        try {
            if (choice == 1) {
                std::string fileName;
                std::cout << "Enter JSON file name: ";
                std::cin >> fileName;

                records = parseData(fileName);
                dataLoaded = true;

                std::cout << "✓ Successfully loaded " 
                          << records.size() << " records.\n";
            }
            else if (choice == 2) {
                if (!dataLoaded) {
                    std::cout << "⚠ Please load data first.\n";
                } else {
                    calculateStatistics(records);
                }
            }
            else if (choice == 3) {
                if (!dataLoaded) {
                    std::cout << "⚠ Please load data first.\n";
                } else {
                    int ageLimit;
                    std::cout << "Filter records with age greater than: ";
                    std::cin >> ageLimit;

                    auto filtered = filterRecordsByPredicate(
                        records,
                        [ageLimit](const DataRecord& r) {
                            return r.age > ageLimit;
                        }
                    );

                    std::cout << "✓ Found " << filtered.size()
                              << " matching records:\n";
                    for (const auto& r : filtered) {
                        std::cout << "  - " << r.name
                                  << " (Age: " << r.age << ")\n";
                    }
                }
            }
            else if (choice == 4) {
                if (!dataLoaded) {
                    std::cout << "⚠ Please load data first.\n";
                } else {
                    normalizeCategoryNames(records);
                    std::cout << "✓ Category names normalized to uppercase.\n";
                }
            }
            else if (choice == 5) {
                if (!dataLoaded) {
                    std::cout << "⚠ Please load data first.\n";
                } else {
                    auto aggregated = aggregateValuesByCategory(records);
                    std::cout << "Category Totals:\n";
                    for (const auto& pair : aggregated) {
                        std::cout << "  - " << pair.first
                                  << ": " << pair.second << "\n";
                    }
                }
            }
            else if (choice == 6) {
                std::cout << "Exiting application. Goodbye!\n";
                break;
            }
            else {
                std::cout << "❌ Invalid option. Try again.\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "❌ Error: " << e.what() << "\n";
        }
    }
    return 0;
}