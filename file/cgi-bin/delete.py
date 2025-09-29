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
	sys.exit(0)
except FileNotFoundError:
	print("Error file not found")
	sys.exit(1)
except PermissionError:
	print("Error permission denied")
	sys.exit(1)
except Exception as e:
	print(f"Error deleting file: {e}")
	sys.exit(1)