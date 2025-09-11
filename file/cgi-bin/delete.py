#!/usr/bin/env python3

import os
import sys

methode = os.environ.get("REQUEST_METHOD", "")

if methode != "DELETE":
	print("Error bad methode use to call the script")
	sys.exit(1)

path = os.environ.get("PATH_INFO")
if not path:
	print("Error no path provided")
	sys.exit(1)

try:
	os.remove(f".{path}")
	print(f"File deleted successfully.")
except FileNotFoundError:
	print("Error file not found")
except PermissionError:
	print("Error permission denied")
except Exception as e:
	print(f"Error deleting file: {e}")