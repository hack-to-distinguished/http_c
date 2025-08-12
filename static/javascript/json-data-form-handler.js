document.getElementById('profileForm').addEventListener('submit', async function(event) {
  event.preventDefault(); // Stop default form submission

  const formData = new FormData(this);
  const jsonData = {};

  // Convert FormData to JSON object
  formData.forEach((value, key) => {
    jsonData[key] = value;
  });

  // Send JSON with fetch
  try {
    const response = await fetch('/submit', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(jsonData)
    });

    const result = await response.text();
    console.log('Server response:', result);
    document.body.innerHTML = result; 
  } catch (err) {
    console.error('Error sending form:', err);
  }
});
