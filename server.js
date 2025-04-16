require('dotenv').config();
const express = require('express');
const { Client } = require('pg'); // Χρησιμοποιούμε το PostgreSQL client
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// Δημιουργία σύνδεσης με τη βάση δεδομένων
const client = new Client({
  host: process.env.DB_HOST,
  port: process.env.DB_PORT,
  database: process.env.DB_NAME,
  user: process.env.DB_USER,
  password: process.env.DB_PASS,
});

client.connect()
  .then(() => {
    console.log('Connected to PostgreSQL Database');
  })
  .catch((err) => {
    console.error('Database connection failed', err.stack);
  });

// API για αποθήκευση δεδομένων
app.post('/data', (req, res) => {
  const { temperature, humidity, pressure, air_quality } = req.body;
  
  const sql = 'INSERT INTO measurements (temperature, humidity, pressure, air_quality) VALUES ($1, $2, $3, $4)';
  const values = [temperature, humidity, pressure, air_quality];

  client.query(sql, values, (err, result) => {
    if (err) {
      console.error('Error saving data:', err);
      return res.status(500).send('Error saving data');
    }
    res.send('Data saved successfully');
  });
});

// API για ανάκτηση δεδομένων
app.get('/data', (req, res) => {
  const sql = 'SELECT * FROM measurements ORDER BY timestamp DESC LIMIT 50';

  client.query(sql, (err, result) => {
    if (err) {
      console.error('Error retrieving data:', err);
      return res.status(500).send('Error retrieving data');
    }
    res.json(result.rows); // Χρησιμοποιούμε `result.rows` για τα αποτελέσματα
  });
});

const path = require('path');
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Εκκίνηση του server
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
