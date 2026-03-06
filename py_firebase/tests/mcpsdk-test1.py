import sys
import json
import os
from typing import Any

import firebase_admin
from firebase_admin import credentials, db
from mcp.server.fastmcp import FastMCP

def initialize_firebase():
    """
    Initializes the Firebase Admin SDK.

    Reads Firebase service account credentials from an environment variable
    `FIREBASE_SERVICE_ACCOUNT_KEY_JSON`. The app is initialized with the
    database URL specified in the credentials.

    Returns:
        bool: True if initialization is successful, False otherwise.
    """
    try:
        # Get the JSON credentials from the environment variable.
        """cred_json_str = os.environ.get('FIREBASE_SERVICE_ACCOUNT_KEY_JSON')
        if not cred_json_str:
            sys.stderr.write("Error: FIREBASE_SERVICE_ACCOUNT_KEY_JSON environment variable not set.\n")
            return False

        cred_json = json.loads(cred_json_str)
        
        # Extract the databaseURL from the credentials if it exists.
        # It's common for RTDB service accounts to not have this, so we use a fallback.
        database_url = cred_json.get('databaseURL', 'https://tarp-aura-default-rtdb.asia-southeast1.firebasedatabase.app/')

        cred = credentials.Certificate(cred_json)
        firebase_admin.initialize_app(cred, {
            'databaseURL': database_url
        })"""
        cred = credentials.Certificate('tarp-aura-firebase-adminsdk-fbsvc-53caaf3006.json')
        firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://tarp-aura-default-rtdb.asia-southeast1.firebasedatabase.app/'
})
        return True
    except (ValueError, json.JSONDecodeError) as e:
        sys.stderr.write(f"Error decoding FIREBASE_SERVICE_ACCOUNT_KEY_JSON: {e}\n")
        return False
    except Exception as e:
        sys.stderr.write(f"Error initializing Firebase: {e}\n")
        return False

# Initialize Firebase before defining the server.
# The script will exit if initialization fails.
if not initialize_firebase():
    sys.exit(1)

# Create a FastMCP server instance
mcp = FastMCP(
    "FirebaseController",
    "Provides tools to interact with a Firebase Realtime Database."
)

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

