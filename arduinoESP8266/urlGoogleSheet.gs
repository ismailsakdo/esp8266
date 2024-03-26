// This function is called when the web app URL is accessed
function doGet(e) {
  // Get current date and time
  var dateTime = new Date();
  
  // Get the event from the query parameter 'event'
  var event = e.parameter.event;
  
  // Append data to the Google Sheet
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  sheet.appendRow([dateTime, event]);
  
  // Send an email
  var emailAddress = "XXXX@gmail.com"; // Change this to the recipient's email address
  var subject = "Event Triggered: " + event;
  var message = "Event '" + event + "' was triggered at " + dateTime + ".";
  MailApp.sendEmail(emailAddress, subject, message);
  
  // Return a simple response
  return ContentService.createTextOutput('Event recorded: ' + event + '. Email sent to ' + emailAddress);
}
