#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <curl/curl.h>
#include <openssl/md5.h>
#include <json/json.h>

using namespace std;
namespace fs = std::filesystem;

// Configuration variables
const string API_URL = "https://api.example.com";
const string API_KEY = "your-api-key-here";
const string SCAN_DIR = "/path/to/scan";
const int SCAN_INTERVAL = 60; // seconds

// Define a struct to hold file information
struct FileInfo {
    string path;
    long size;
    string md5;
};

// Define a struct to hold scan results
struct ScanResult {
    bool is_infected;
    string virus_name;
};

// Define a function to calculate the MD5 hash of a file
string md5_hash_file(const string& file_path) {
    ifstream file(file_path, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file: " + file_path);
    }

    MD5_CTX md5_context;
    MD5_Init(&md5_context);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)).gcount() > 0) {
        MD5_Update(&md5_context, buffer, file.gcount());
    }

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &md5_context);

    stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << static_cast<unsigned>(hash[i]);
    }

    return ss.str();
}

// Define a function to scan a file for viruses using Microsoft Defender API
ScanResult scan_file_with_defender(const string& file_path) {
    ScanResult result;
    result.is_infected = false;

    // Prepare HTTP request
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize cURL");
    }

    string api_key = "your-defender-api-key-here";
    string api_version = "v1.0";
    string endpoint = "https://api-us.securitycenter.microsoft.com/api/" + api_version + "/file/";
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    // Set request headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set request body
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, file_path.c_str(), CURLFORM_END);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    // Receive response as string
    string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* userdata) {
        size_t num_bytes = size * nmemb;
        string& response = *reinterpret_cast<string*>(userdata);
        response.append(reinterpret_cast<char*>(ptr), num_bytes);
        return num_bytes;
        });

