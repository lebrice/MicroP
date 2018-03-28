
import requests
from requests import put, get
import json
fake_data = {
    "data": {
        "x": [1,2,3,4,5,6,7],
        "y": [1,2,3,4,5,6,7],
        "z": [1,2,3,4,5,6,7],
    }
}

fake_data = {
    "data": 3
}


response = put('http://localhost:5000/accelerometer/0',json=fake_data)
print(response)
print(response.json())