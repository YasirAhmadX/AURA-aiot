import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

# Fetch the service account key JSON file contents
cred = credentials.Certificate('tarp-aura-firebase-adminsdk-fbsvc-53caaf3006.json')

# Initialize the app with a service account, granting admin privileges
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://tarp-aura-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# As an admin, the app has access to read and write all data, regradless of Security Rules
ref = db.reference('relay1')
print(f"relay state: {ref.get()}")

ref = db.reference('led')
print(f"led state: {ref.get()}")

ref = db.reference('ulsc')
print(f"ulsc state: {ref.get()}")

ref = db.reference('led')
print(f"led state changed to False: {ref.set(False)}")

ref = db.reference('led')
print(f"led state: {ref.get()}")