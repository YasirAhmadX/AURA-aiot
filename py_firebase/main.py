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

# --- MODIFICATION 1: Updated Server Description ---
# Create a FastMCP server instance
mcp = FastMCP(
    "HomeAutomationController",
    "Provides tools to control and monitor Home Automation (IoT) devices "
    "like lights, relays, and sensors connected to a Firebase Realtime Database."
)
sys.stderr.write("FastMCP server instance created.\n")

@mcp.tool()
def get_state(device: str) -> dict:
    # --- MODIFICATION 2: Updated Tool Description ---
    """
    Get the current state of a specific IoT device (e.g., 'led', 'relay1', 'temperature').
    The state could be a boolean (true/false) for a light or a numerical value for a sensor.

    Args:
        device: The name/path of the device in the database (e.g., 'relay1', 'temperature').
    """
    try:
        ref = db.reference(device)
        state = ref.get()
        return {"device": device, "state": state}
    except Exception as e:
        return {"error": f"Failed to get state for {device}: {e}"}

@mcp.tool()
def set_state(device: str, state: Any) -> dict:
    # --- MODIFICATION 3: Updated Tool Description ---
    """
    Set a new state for a specific IoT device. For example, 'led' to true to
    turn on the light, or 'relay1' to false to turn it off.

    Args:
        device: The name/path of the device in the database (e.g., 'led', 'relay1').
        state: The new state to set (e.g., True, False, 0, 1, or a string).
    """
    try:
        ref = db.reference(device)
        ref.set(state)
        return {"device": device, "new_state": state, "status": "success"}
    except Exception as e:
        return {"error": f"Failed to set state for {device}: {e}"}

# --- MODIFICATION 4: New Tool ---
@mcp.tool()
def get_device_purposes() -> dict:
    """
    Gets the configuration mapping of all available device IDs to their 
    real-world purpose or description (e.g., 'led' -> 'bedroom light', 'relay1' -> 'hall light').
    This is the first tool an agent should call to understand what devices it can control.
    """
    try:
        ref = db.reference("config")
        config_data = ref.get()
        if config_data:
            return {"status": "success", "device_purposes": config_data}
        else:
            return {"status": "error", "message": "The 'config' node was not found or is empty."}
    except Exception as e:
        return {"error": f"Failed to get device purposes from 'config': {e}"}


# Add a final message to confirm the script has loaded
# and is now waiting for MCP messages from stdin
sys.stderr.write("Firebase MCP server definitions loaded. Awaiting MCP messages...\n")

# This is the missing line.
# mcp.run() starts the server's main loop, listening for
# messages from stdin and preventing the script from exiting.
try:
    sys.stderr.write("Starting FastMCP server run loop...\n")
    mcp.run()
except Exception as e:
    sys.stderr.write(f"Critical error during MCP run loop: {e}\n")
    sys.exit(1)