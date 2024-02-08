from fastapi import FastAPI, BackgroundTasks, Depends
from fastapi.responses import JSONResponse
import json
import asyncio

app = FastAPI()

def fetch_uart_data():
    raw_data = "Strawberry-60-70-18.00-400-10-20-30"
    return raw_data

# Function to parse the raw data
def parse_uart_data(raw_data):
    plant_name, humidity, temperature, soil_moisture, air_quality, nitrogen, phosphorus, potassium = raw_data.split('-')
    return {
        "plant_name": plant_name,
        "humidity": float(humidity),
        "temperature": float(temperature),
        "soil_moisture": float(soil_moisture),
        "air_quality": int(air_quality),
        "nitrogen": int(nitrogen),
        "phosphorus": int(phosphorus),
        "potassium": int(potassium),
    }

async def update_uart_data():
    while True:
        raw_data = fetch_uart_data()
        parsed_data = parse_uart_data(raw_data)
        app.state.uart_data = parsed_data
        await asyncio.sleep(5)

@app.get("/uart-data")
async def get_uart_data():
    return app.state.uart_data

@app.on_event("startup")
async def startup_event():
    app.state.uart_data = {}
    asyncio.create_task(update_uart_data())

@app.get("/")
def read_hello():
    content = {"message": "Hello, World!"}
    return JSONResponse(content=content)