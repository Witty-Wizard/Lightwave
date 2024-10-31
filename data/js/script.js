// Initialize Materialize components
document.addEventListener("DOMContentLoaded", function () {
  // Initialize tabs
  var tabsElems = document.querySelectorAll(".tabs");
  M.Tabs.init(tabsElems);
  const toggleButton = document.getElementById("toggleButton");
  let isOn = true;
  fetch("/toggleGet", {
    method: "GET", // Use GET since we're not sending any data
    headers: {
      "Content-Type": "application/json",
    },
  }).then((response) => response.json())
  .then((data) => {
    isOn = data.isOn;

    toggleButton.textContent = isOn ? "On" : "Off";
    toggleButton.classList.toggle("green-button", isOn);
    toggleButton.classList.toggle("red-button", !isOn);
  })
  .catch((error) => {
    console.error("Error:", error);
  });
  toggleButton.textContent = isOn ? "On" : "Off";

  // Initialize time pickers
  var timepickerElems = document.querySelectorAll(".timepicker");
  M.Timepicker.init(timepickerElems, {
    twelveHour: true,
    defaultTime: "now",
    autoClose: true,
  });

  toggleButton.classList.add("green-button");

  toggleButton.addEventListener("click", function () {
    fetch("/toggle", {
      method: "GET", // Use GET since we're not sending any data
      headers: {
        "Content-Type": "application/json",
      },
    })
      .then((response) => response.json())
      .then((data) => {
        isOn = data.isOn;

        toggleButton.textContent = isOn ? "On" : "Off";
        toggleButton.classList.toggle("green-button", isOn);
        toggleButton.classList.toggle("red-button", !isOn);
      })
      .catch((error) => {
        console.error("Error:", error);
      });
  });

  // Initialize the clock
  updateClock();
  setInterval(updateClock, 1000);

  // Hide loading screen and show content
  document.getElementById("loadingOverlay").style.display = "none";
  const content = document.getElementById("content");
  content.style.visibility = "visible";
  content.style.opacity = "1"; // Fade in the content
});

// Wi-Fi Form Submission
document
  .getElementById("wifiForm")
  .addEventListener("submit", function (event) {
    event.preventDefault();
    const ssid = document.getElementById("ssid").value;
    const password = document.getElementById("password").value;

    fetch("/connect", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ ssid, password }),
    })
      .then((response) => response.text())
      .then((data) => {
        document.getElementById("wifiResponse").innerText = data;
      })
      .catch((error) => {
        console.error("Error:", error);
        document.getElementById("wifiResponse").innerText =
          "Error: Unable to connect";
      });
  });

// General Setup Form Submission
document
  .getElementById("setupForm")
  .addEventListener("submit", function (event) {
    event.preventDefault();
    const onTimeString = document.getElementById("onTime").value;
    const offTimeString = document.getElementById("offTime").value;

    if (!onTimeString || !offTimeString) {
      document.getElementById("setupResponse").innerText =
        "Please fill out all fields.";
      return;
    }

    // Convert onTime and offTime to epoch times
    const onTime = convertToEpoch(onTimeString);
    const offTime = convertToEpoch(offTimeString);

    fetch("/setup", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ onTime, offTime }),
    })
      .then((response) => response.text())
      .then((data) => {
        document.getElementById("setupResponse").innerText = data;
      })
      .catch((error) => {
        console.error("Error:", error);
        document.getElementById("setupResponse").innerText =
          "Error: Unable to save settings";
      });
  });

// Set Current Time Form Submission
document
  .getElementById("timeForm")
  .addEventListener("submit", function (event) {
    event.preventDefault();
    const currentTimeString = document.getElementById("currentTime").value;

    if (!currentTimeString) {
      document.getElementById("setupResponse").innerText =
        "Please fill out all fields.";
      return;
    }

    const currentTime = convertToEpoch(currentTimeString);

    fetch("/setTime", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ currentTime }),
    })
      .then((response) => response.text())
      .then((data) => {
        document.getElementById("timeResponse").innerText = data;
      })
      .catch((error) => {
        console.error("Error:", error);
        document.getElementById("timeResponse").innerText =
          "Error: Unable to set time";
      });
  });

// Function to update the clock
function updateClock() {
  const clockElement = document.getElementById("digitalClock");
  const now = new Date();
  let hours = now.getHours();
  const minutes = now.getMinutes().toString().padStart(2, "0");
  const ampm = hours >= 12 ? "PM" : "AM";
  hours = hours % 12 || 12; // Convert to 12-hour format

  clockElement.innerHTML = `${hours}<span class="blinking-colon">:</span>${minutes} ${ampm}`;
}

function convertToEpoch(timeString) {
  const [time, modifier] = timeString.split(" ");
  let [hours, minutes] = time.split(":");
  hours = parseInt(hours, 10);

  // Adjust hours for AM/PM
  if (modifier === "PM" && hours !== 12) hours += 12;
  if (modifier === "AM" && hours === 12) hours = 0;

  // Create a new Date object with today's date and the parsed time
  const now = new Date();
  const dateWithTime = new Date(
    now.getFullYear(),
    now.getMonth(),
    now.getDate(),
    hours,
    parseInt(minutes, 10)
  );

  const timezoneOffset = dateWithTime.getTimezoneOffset() * 60;

  // Convert to epoch time in seconds
  return Math.floor(dateWithTime.getTime() / 1000) - timezoneOffset;
}
