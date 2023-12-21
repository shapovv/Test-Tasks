$(document).ready(function() {
    // Инициализация календаря
    initializeCalendar();

    // Установка обработчиков событий
    setupEventHandlers();

    // Загрузка задач на сегодняшний день
    fetchTasksForToday();
});

function initializeCalendar() {
    // Инициализация jQuery UI datepicker для календаря
    //...
}

function setupEventHandlers() {
    // Обработчик события для поиска задач
    $('#search-box').on('keyup', handleSearch);

    // Остальные обработчики событий...
    //...
}

function handleSearch(event) {
    var searchText = $(this).val().toLowerCase();
    var url = 'http://localhost:3000/api/todos/find?q=' + encodeURIComponent(searchText);


}


function fetchTasksForToday() {
    var today = new Date().toISOString().split('T')[0];
    var url = 'http://localhost:3000/api/todos/date?start=' + today + '&end=' + today;



    $.ajax({
        url: url,
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
        },
        success: function(response) {
            displayTasks(response);
        },
        error: function(xhr, status, error) {
            console.error("Error fetching tasks for today:", error);
        }
    });
}

function fetchTasksForWeek() {
    var currentDate = new Date();
    var startOfWeek = new Date(currentDate.setDate(currentDate.getDate() - currentDate.getDay() + (currentDate.getDay() === 0 ? -6 : 1))).toISOString().split('T')[0];
    var endOfWeek = new Date(currentDate.setDate(currentDate.getDate() - currentDate.getDay() + 7)).toISOString().split('T')[0];
    var url = 'http://localhost:3000/api/todos/date?start=' + startOfWeek + '&end=' + endOfWeek;



    $.ajax({
        url: url,
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
        },
        success: function(response) {
            displayTasks(response);
        },
        error: function(xhr, status, error) {
            console.error("Error fetching tasks for the week:", error);
        }
    });
}




function fetchTasks() {
    // URL API для получения полного списка задач
    var url = 'http://localhost:3000/api/todos';

    $.ajax({
        url: url,
        method: 'GET',
        headers: {
            'accept': 'application/json',
        },
        success: function(response) {
            // Отображаем задачи, используя полученные данные
            displayTasks(response);
        },
        error: function(xhr, status, error) {
            console.error("Error fetching tasks:", error);
        }
    });
}

// Инициализация и загрузка всех задач при запуске
$(document).ready(function() {
    fetchTasks(); // Загрузка всех задач
});



// Инициализация и загрузка задач при запуске
$(document).ready(function() {
    fetchTasks(); // Загрузка всех задач без фильтра поиска
});


function displayTasks(tasks) {
    var tasksContainer = $('#tasks-container');
    tasksContainer.empty(); // Очищаем контейнер задач

    tasks.forEach(function(task) {
        // Создаем элементы для отображения задачи
        var taskElement = $('<div class="task"></div>').attr('id', task.id);
        var titleElement = $('<div class="task-title"></div>').text(task.name);
        var shortDescElement = $('<div class="task-short-desc"></div>').text(task.shortDesc);
        var dateElement = $('<div class="task-date"></div>').text(new Date(task.date).toLocaleString());
        var statusElement = $('<div class="task-status"></div>').text(task.status ? 'Выполнено' : 'Не выполнено');

        // Добавляем элементы в элемент задачи
        taskElement.append(titleElement, shortDescElement, dateElement, statusElement);

        // Добавляем элемент задачи в контейнер задач
        tasksContainer.append(taskElement);
    });
}


