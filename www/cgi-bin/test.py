#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("<p>Request Method: " + os.environ.get("REQUEST_METHOD", "") + "</p>")
print("<p>Query String: " + os.environ.get("QUERY_STRING", "") + "</p>")
print("</body></html>")