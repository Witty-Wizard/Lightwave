// Initialize Materialize components
document.addEventListener("DOMContentLoaded", function () {
  // Initialize tabs
  var tabsElems = document.querySelectorAll(".tabs");
  M.Tabs.init(tabsElems);

  // Initialize time pickers
  var timepickerElems = document.querySelectorAll(".timepicker");
  M.Timepicker.init(timepickerElems, {
    twelveHour: true,
    defaultTime: "now",
    autoClose: true,
  });

  // Initialize the clock
  updateClock();
  setInterval(updateClock, 1000); // Update the clock every second
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
    const onTime = document.getElementById("onTime").value;
    const offTime = document.getElementById("offTime").value;

    if (!onTime || !offTime) {
      document.getElementById("setupResponse").innerText =
        "Please fill out all fields.";
      return;
    }

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
    const currentTime = document.getElementById("currentTime").value;

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
