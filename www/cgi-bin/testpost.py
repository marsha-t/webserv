#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r\n")

print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("<p>Request Method: " + os.environ.get("REQUEST_METHOD", "") + "</p>")
print("<p>Query String: " + os.environ.get("QUERY_STRING", "") + "</p>")

# If it's a POST request, read and display body
if os.environ.get("REQUEST_METHOD", "") == "POST":
    length = os.environ.get("CONTENT_LENGTH")
    if length:
        try:
            length = int(length)
            body = sys.stdin.read(length)
            print("<h2>POST Body</h2>")
            print("<pre>" + body + "</pre>")
        except Exception as e:
            print("<p>Error reading POST body: {}</p>".format(str(e)))

print("</body></html>")
