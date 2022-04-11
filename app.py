import datetime
import requests
from fastapi import FastAPI

app = FastAPI()


API_KEY = ""

def get_vasttrafik_auth():
    headers = {
        "content-type": "application/x-www-form-urlencoded",
        "Authorization": "Basic " + API_KEY
    }
    return requests.post("https://api.vasttrafik.se/token?grant_type=client_credentials", headers=headers).json()["access_token"]


def get_departures(stop_id):
    headers = {
        "Authorization": "Bearer " + get_vasttrafik_auth()
    }
    url = f"https://api.vasttrafik.se/bin/rest.exe/v2/departureBoard?id={stop_id}&format=json&timeSpan=15"
    return requests.get(url, headers=headers).json()["DepartureBoard"]["Departure"]

def get_name_of_stop(stop_id):
    headers = {
        "Authorization": "Bearer " + get_vasttrafik_auth()
    }
    url = f"https://api.vasttrafik.se/bin/rest.exe/v2/departureBoard?id={stop_id}&format=json&timeSpan=15"
    return requests.get(url, headers=headers).json()["DepartureBoard"]["Departure"][0]["stop"]

def convert_hex(hexcolor):
    hexcolor = hexcolor.lstrip("#").upper()
    R, G, B = tuple(int(hexcolor[i:i+2], 16) for i in (0, 2, 4))
    return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) )


def time_diffrence_now_and_time(time):
    time = datetime.datetime.strptime(time, "%H:%M")
    time = time.replace(year=datetime.datetime.now().year, month=datetime.datetime.now().month, day=datetime.datetime.now().day)
    diffrence = (time - datetime.datetime.now()).total_seconds() / 60 if time > datetime.datetime.now() else (time - datetime.datetime.now()).total_seconds() / 60 + 24 * 60
    return int(diffrence)

def retrun_departure_times(stop_id = "9021014003200000"):
    all_departures = get_departures(stop_id)
    all_departures_list = []
    for departure in all_departures:
        if departure["type"] == "TRAM" and len(departure["sname"]) == 1 and time_diffrence_now_and_time(departure["time"]) <= 30:
            departure_data = {"line": departure["sname"],
                            "time": departure["time"],
                            "time_diffrence_min": time_diffrence_now_and_time(departure["time"]) ,
                            "direction": departure["direction"][:9].replace("ö", "o").replace("ä", "a").replace("å", "a"),
                            "bgColor": convert_hex(departure["bgColor"]),
                            "fgColor": convert_hex(departure["fgColor"])}
        
            all_departures_list.append(departure_data)
    return all_departures_list
        
    
    
@app.get("/get_times/{stop_id}")
@app.get("/get_times")
def get_times(stop_id = "9021014003200000"):
    return retrun_departure_times(str(stop_id))

@app.get("/get_name/{stop_id}")
@app.get("/get_name")
def get_name(stop_id = "9021014003200000"):
    data = {"stop_name" : get_name_of_stop(stop_id).replace("ö", "o").replace("ä", "a").replace("å", "a")}
    return data

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000, debug=True)