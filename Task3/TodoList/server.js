const express = require('express');
const cors = require('cors');
const axios = require('axios');
const app = express();

app.use(cors());

// Маршрут для получения всех задач
app.get('/api/todos', async (req, res) => {
    try {
        const response = await axios.get('https://todo.doczilla.pro/api/todos');
        res.json(response.data);
    } catch (error) {
        res.status(500).send(error.toString());
    }
});

// Маршрут для получения задач по дате
app.get('/api/todos/date', async (req, res) => {
    const { start, end } = req.query; // Извлекаем параметры запроса
    try {
        const response = await axios.get(`https://todo.doczilla.pro/api/todos/date?start=${start}&end=${end}`);
        res.json(response.data);
    } catch (error) {
        res.status(500).send(error.toString());
    }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});
