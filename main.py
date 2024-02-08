from fastapi import FastAPI, WebSocket
from fastapi.responses import JSONResponse
import json
import asyncio
import cv2
import serial

app = FastAPI()

def fetch_uart_data():
    try:
        with serial.Serial('/dev/tty1', baudrate=9600, timeout=1) as uart_port:
            uart_data = uart_port.readline().decode('utf-8').strip()
        return uart_data
    except serial.SerialException as e:
        print(f"Error reading UART data: {e}")
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
        
        async with app.state.uart_data_lock:
            app.state.uart_data = parsed_data

        await asyncio.sleep(5)

@app.get("/uart_data")
async def get_uart_data():
    async with app.state.uart_data_lock:
        return app.state.uart_data

@app.on_event("startup")
async def startup_event():
    app.state.uart_data = {}
    app.state.uart_data_lock = asyncio.Lock()
    asyncio.create_task(update_uart_data())

async def capture_video(websocket: WebSocket):
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        error_message = "Error: Could not open camera."
        await websocket.send_text(error_message)
        return

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            _, buffer = cv2.imencode('.jpg', frame)
            img_str = buffer.tobytes()

            await websocket.send_bytes(img_str)

            await asyncio.sleep(0.1)
    finally:
        cap.release()

@app.websocket("/live_camera")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    await capture_video(websocket)

@app.get("/")
def read_hello():
    content = {"message": "Hello, World!"}
    return JSONResponse(content=content)