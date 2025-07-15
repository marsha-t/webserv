#include "../includes/Request.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// C++98 compatible helper function
std::string build_multipart_body(const std::string& boundary, const std::vector<std::pair<std::string, std::string> >& parts, std::string& body) {
    body.clear();
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parts.begin(); it != parts.end(); ++it) {
        body += "--" + boundary + "\r\n";
        body += it->first;
        body += "\r\n\r\n";
        body += it->second;
        body += "\r\n";
    }
    body += "--" + boundary + "--\r\n";
    
    std::stringstream ss;
    ss << body.length();
    return ss.str();
}

void test_simple_get() {
    Request req;
    std::string raw_request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    assert(req.parse(raw_request));
    assert(req.getMethod() == "GET");
    assert(req.getTarget() == "/index.html");
    assert(req.getVersion() == "HTTP/1.1");
    std::cout << "test_simple_get PASSED" << std::endl;
}

void test_post_urlencoded() {
    Request req;
    std::string raw_request = "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 11\r\n\r\nkey1=value1";
    assert(req.parse(raw_request));
    req.validateBody(1000000);
    assert(req.getMethod() == "POST");
    assert(req.getFormData().size() == 1);
    assert(req.getFormData().at("key1") == "value1");
    std::cout << "test_post_urlencoded PASSED" << std::endl;
}

void test_multipart_upload() {
    Request req;
    std::string boundary = "----boundary";
    std::vector<std::pair<std::string, std::string> > parts;
    parts.push_back(std::make_pair("Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"", "file content"));
    std::string body;
    std::string content_length = build_multipart_body(boundary, parts, body);

    std::string raw_request =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "Content-Length: " + content_length + "\r\n"
        "\r\n" + body;

    assert(req.parse(raw_request));
    req.validateBody(1000000);
    assert(req.getMethod() == "POST");
    assert(req.getUploadedFiles().size() == 1);
    assert(req.getUploadedFiles().count("test.txt"));
    assert(req.getUploadedFiles().at("test.txt") == "file content");
    std::cout << "test_multipart_upload PASSED" << std::endl;
}

void test_chunked_multipart() {
    Request req;
    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::vector<std::pair<std::string, std::string> > parts;
    parts.push_back(std::make_pair("Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\nContent-Type: text/plain", "this is a test file\n"));
    std::string body;
    build_multipart_body(boundary, parts, body);

    std::stringstream chunked_body_ss;
    chunked_body_ss << std::hex << body.length();
    std::string chunked_body = chunked_body_ss.str() + "\r\n" + body + "\r\n0\r\n\r\n";

    std::string raw_request =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "\r\n" + chunked_body;

    assert(req.parse(raw_request) && "Request::parse should succeed");
    assert(req.validateBody(1000000) && "Chunked multipart request should be valid");
    assert(req.getUploadedFiles().count("test.txt") > 0 && "File should be uploaded");
    assert(req.getUploadedFiles().at("test.txt") == "this is a test file\n");
    std::cout << "test_chunked_multipart PASSED" << std::endl;
}

int main() {
    test_simple_get();
    test_post_urlencoded();
    test_multipart_upload();
    test_chunked_multipart();
    return 0;
}