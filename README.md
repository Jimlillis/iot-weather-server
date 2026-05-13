# IoT Weather Server

A full-stack IoT weather station system that collects atmospheric sensor data from an Arduino device, stores it in a PostgreSQL database, and displays it on a live web dashboard.

Live deployment: [iot-weather-server.onrender.com](https://iot-weather-server.onrender.com)

---

## Overview

The system has two main parts:

- **Arduino firmware** (`Weather_Station_DimAl.ino`) — reads sensors and sends measurements to the server every few seconds, while polling for control commands to execute locally (LED signals).
- **Node.js server** (`server.js`) — receives and stores measurements, serves the web dashboard, and relays control commands back to the Arduino.

---

## Hardware

| Component | Role | Pin |
|-----------|------|-----|
| Arduino (WiFiS3) | Main microcontroller | — |
| DHT22 | Temperature & humidity sensor | D2 |
| MQ135 | Air quality sensor (analog) | A0 |
| Water level sensor | Water/pressure level (analog) | A5 |
| LED — Temperature | Visual feedback | D6 |
| LED — Humidity | Visual feedback | D5 |
| LED — Water | Visual feedback | D4 |
| LED — Air Quality | Visual feedback | D3 |

The Arduino connects over WiFi (SSL) to the Render-hosted server and:
1. POSTs a JSON measurement (`temperature`, `humidity`, `pressure`, `air_quality`) to `/data`
2. GETs `/command` to check for a pending control command and blinks the corresponding LED for 2 seconds

---

## Server

Built with **Node.js** and **Express**, backed by **PostgreSQL**.

### API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/data` | Receive sensor measurement from Arduino (no auth required) |
| `GET` | `/data` | Return last 50 measurements — requires login session |
| `POST` | `/login` | Authenticate user, create session |
| `POST` | `/logout` | Destroy session |
| `POST` | `/command` | Queue a control command for the Arduino (`temperature`, `humidity`, `water`, `air_quality`) |
| `GET` | `/command` | Arduino polls this to retrieve and clear the pending command |
| `GET` | `/` | Redirects to login page |

### Database

A single PostgreSQL table is created automatically on startup:

```sql
CREATE TABLE IF NOT EXISTS measurements (
  id        SERIAL PRIMARY KEY,
  temperature FLOAT,
  humidity    FLOAT,
  pressure    FLOAT,
  air_quality FLOAT,
  timestamp   TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

> **Note:** The `pressure` column stores water level sensor readings (scaled to a 0–100 range), not atmospheric pressure.

---

## Web Dashboard

After login, the dashboard (`new_index_2.html`) shows four sensor cards — Temperature, Humidity, Water Level, and Air Quality — each with:

- Current value (updated on page refresh)
- Historical carousel of the last 8 readings
- ApexCharts line chart
- A control button that sends a command to the Arduino (lights the corresponding LED)

The **Refresh** button fetches the latest data from `/data` and updates all displayed values.

---

## Running Locally

### Prerequisites

- Node.js
- A PostgreSQL database
- An `.env` file in the project root:

```env
DB_HOST=your_db_host
DB_PORT=5432
DB_NAME=your_db_name
DB_USER=your_db_user
DB_PASS=your_db_password
SESSION_SECRET=your_session_secret
PORT=5000
```

### Start the server

```bash
npm install
npm start
```

The server will connect to PostgreSQL, create the `measurements` table if it doesn't exist, and listen on the configured port.

---

## Deployment

The server is deployed on **Render.com**. The PostgreSQL database connection uses SSL with `rejectUnauthorized: false` as required by Render's managed database.

The Arduino firmware is hardcoded to connect to `iot-weather-server.onrender.com` on port 443 (HTTPS).

---

## Tech Stack

| Layer | Technology |
|-------|------------|
| Hardware | Arduino UNO R4 WiFi, DHT22, MQ135 |
| Backend | Node.js, Express, express-session |
| Database | PostgreSQL (via `pg`) |
| Frontend | Vanilla HTML/CSS/JavaScript, ApexCharts |
| Deployment | Render.com |
