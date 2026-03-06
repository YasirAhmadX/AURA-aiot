import sys
import json
import os
from typing import Any

import firebase_admin
from firebase_admin import credentials, db
from mcp.server.fastmcp import FastMCP

# --- Start of Changes ---

# Get the directory where the main.py script is located
# This makes file paths much more reliable
try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
except NameError:
    # Fallback if __file__ is not defined (e.g., in some interactive environments)
    script_dir = os.getcwd()
    sys.stderr.write(f"Warning: __file__ not defined. Using current working directory: {script_dir}\n")

# Build the full, absolute path to the credentials file
# This assumes the .json file is in the SAME directory as your main.py
cred_path = os.path.join(script_dir, 'tarp-aura-firebase-adminsdk-fbsvc-53caaf3006.json')

# --- End of Changes ---

def initialize_firebase():
    """
    Initializes the Firebase Admin SDK.
    ... (rest of your docstring) ...
    """
    try:
        # --- (Your commented-out environment variable block) ---
        """cred_json_str = os.environ.get('FIREBASE_SERVICE_ACCOUNT_KEY_JSON')
        if not cred_json_str:
            sys.stderr.write("Error: FIREBASE_SERVICE_ACCOUNT_KEY_JSON environment variable not set.\n")
            return False
        ..."""

        # --- Updated Code ---
        
        # Add a check to see if the file exists at the path we built
        sys.stderr.write(f"Attempting to load credentials from: {cred_path}\n")
        if not os.path.exists(cred_path):
            sys.stderr.write(f"Error: Credentials file not found at the expected path.\n")
            return False

        # Use the absolute cred_path variable
        cred = credentials.Certificate(cred_path)
        firebase_admin.initialize_app(cred, {
            'databaseURL': 'https://tarp-aura-default-rtdb.asia-southeast1.firebasedatabase.app/'
        })
        
        # Add a success message to stderr so you see it in the logs
        sys.stderr.write("Firebase initialized successfully.\n")
        return True
    except (ValueError, json.JSONDecodeError) as e:
        sys.stderr.write(f"Error decoding Firebase credentials: {e}\n")
        return False
    except Exception as e:
        # Add the path to the error message for better debugging
        sys.stderr.write(f"Error initializing Firebase (Path: {cred_path}): {e}\n")
        return False

# Initialize Firebase before defining the server.
# The script will exit if initialization fails.
sys.stderr.write("Attempting to initialize Firebase...\n")
if not initialize_firebase():
    sys.stderr.write("Firebase initialization failed. Exiting.\n")
    sys.exit(1)

# Create a FastMCP server instance
mcp = FastMCP(
    "FirebaseController",
    "Provides tools to interact with a Firebase Realtime Database."
)
sys.stderr.write("FastMCP server instance created.\n")

@mcp.tool()
def get_state(device: str) -> dict:
    """
    Get the state of a specific device from Firebase RTDB.

    Args:
        device: The name/path of the device in the database (e.g., 'relay1').
    """
    try:
        ref = db.reference(device)
        state = ref.get()
        return {"device": device, "state": state}
    except Exception as e:
        return {"error": f"Failed to get state for {device}: {e}"}

@mcp.tool()
def set_state(device: str, state: Any) -> dict:
    """
    Set the state of a specific device in Firebase RTDB.

    Args:
        device: The name/path of the device in the database (e.g., 'led').
        state: The new state to set for the device (e.g., True, False, 0, 1).
    """
    try:
        ref = db.reference(device)
        ref.set(state)
        return {"device": device, "new_state": state, "status": "success"}
    except Exception as e:
        return {"error": f"Failed to set state for {device}: {e}"}

# Add a final message to confirm the script has loaded
# and is now waiting for MCP messages from stdin
sys.stderr.write("Firebase MCP server definitions loaded. Awaiting MCP messages...\n")

# --- START OF NEW CODE ---
# This is the missing line.
# mcp.run() starts the server's main loop, listening for
# messages from stdin and preventing the script from exiting.
try:
    sys.stderr.write("Starting FastMCP server run loop...\n")
    mcp.run()
except Exception as e:
    sys.stderr.write(f"Critical error during MCP run loop: {e}\n")
    sys.exit(1)
# --- END OF NEW CODE ---