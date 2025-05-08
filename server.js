require('dotenv').config();
const express = require('express');
const { Client } = require('pg');
const cors = require('cors');
const path = require('path');
const session = require('express-session');

const app = express();
app.use(cors());
app.use(express.json());

//use session for checking users of the app
app.use(session({
  secret: 'your-secret-key', 
  resave: false,
  saveUninitialized: true,
  cookie: { secure: false } 
}));

console.log("Loaded DB_HOST:", process.env.DB_HOST);

const client = new Client({
  host: process.env.DB_HOST,
  port: process.env.DB_PORT,
  database: process.env.DB_NAME,
  user: process.env.DB_USER,
  password: process.env.DB_PASS,
  ssl: {
    rejectUnauthorized: false
  }
});

app.post('/data', async (req, res) => {
  const { temperature, humidity, pressure, air_quality } = req.body;
  const sql = 'INSERT INTO measurements (temperature, humidity, pressure, air_quality) VALUES ($1, $2, $3, $4)';
  const values = [temperature, humidity, pressure, air_quality];

  try {
    await client.query(sql, values);
    res.send('Data saved successfully');
  } catch (err) {
    console.error('Error saving data:', err);
    res.status(500).send('Error saving data');
  }
});

app.use(express.static(path.join(__dirname, 'public')));
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'login.html'));
});

app.post('/login', (req, res) => {
  const { username, password } = req.body;
  if ((username === 'alexandroskosmidis' && password === '12345')|| (username == 'jim_lillis_junior' && password == 'airdripler')) {
    req.session.authenticated = true;
    res.sendStatus(200);
  } else {
    res.sendStatus(401);
  }
});

app.get('/data', async (req, res) => {
  if (!req.session.authenticated) {
    return res.status(401).json({ message: 'Δεν έχετε συνδεθεί. Παρακαλώ κάντε login πρώτα.' });
  } else{
  try {
    const result = await client.query('SELECT * FROM measurements ORDER BY timestamp DESC LIMIT 50');
    res.json(result.rows);
  } catch (err) {
    console.error('Error retrieving data:', err);
    res.status(500).send('Error retrieving data');
  }
  }
});

async function init() {
  try {
    console.log("Connecting to host:", process.env.DB_HOST);
    await client.connect();
    console.log('Connected to PostgreSQL !');

    await client.query(`
      CREATE TABLE IF NOT EXISTS measurements (
        id SERIAL PRIMARY KEY,
        temperature FLOAT,
        humidity FLOAT,
        pressure FLOAT,
        air_quality FLOAT,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
      );
    `);
    console.log('Table checked/created');

    const PORT = process.env.PORT || 5000;
    app.listen(PORT, () => {
      console.log(`Server running on port ${PORT}`);
    });
  } catch (err) {
    console.error('Initialization error:', err);
  }
}

init();
