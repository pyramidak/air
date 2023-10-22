String header;      // Variable to store the HTTP request
unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0;       // Previous time
const long timeoutTime = 2000;        // Define timeout in milliseconds

void web_server() {
  WiFiClient client = server.available();   // Listen for incoming clients server.available()

  if (client) {                             // If a new client connects,
    report("WebClient connected", true);    // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                  // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            web_request(header);
            web_page(client, header);
                                        
            client.println("</body></html>");         
            client.println(); // The HTTP response ends with another blank line
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    report("WebClient disconnected");
  }
}

int setOption(int def, String name, String msg, int min, int max, int pos, bool restart) {
  int value = findSubmit(name).toInt();
  if (value >= min and value <= max) {
    mem.write(value, pos);
    report("New " + msg + ": " + value);
    if (restart == true) {web_restartNeeded = true;}
    return value;
  } else {
    return def;
  }
}   

String setOption(String def, String name, String msg, int max, int from, int to, bool restart) {
  String value = findSubmit(name);
  if (value != "" and value.length() <= max) {
    mem.write(value, from, to);
    report("New " + msg + ": " + value);
    if (restart == true) {web_restartNeeded = true;}
    return value;
  } else {
    return def;
  }   
}        

void createButton(WiFiClient client, String title, String link) {
  client.println("<p><a href=\"/" + link + "\"><button class=\"button\">" + title + "</button></a></p>");  
}

void createMenu(WiFiClient client, String title, String link, bool paragraph) {
  if (paragraph) {
    client.println("<p><a href=\"/" + link + "\"><button style=width:150px;height:34px;font-size:100%;padding:8px 40px; class=\"button\">" + title + "</button></a></p>");
  } else {
    client.println("<a href=\"/" + link + "\"><button style=width:150px;height:34px;font-size:100%;padding:8px 40px; class=\"button\">" + title + "</button></a>");
  }
}  

void createMenu(WiFiClient client, String title, String link) {
  createMenu(client, title, link, true);
}  

void createSubmit(WiFiClient client, String title, int state, String hint, String link, int maxlength, String description) {
  createSubmit(client, title, String(state), hint, link, maxlength, description);
}  

void createSubmit(WiFiClient client, String title, String state, String hint, String link, int maxlength, String description) {
  client.println("<p>" + title + ": " + state);
  if (description == "") {
    client.println("</p>");
  } else {
    client.println("<br>" + description + "</p>");
  }
  client.println("<FORM ACTION=\"/get\">"); 
  client.println(hint + ": <INPUT TYPE=TEXT NAME=\"" + link + "\" VALUE=\"\" SIZE=\"" + String(maxlength + 2) + "\" MAXLENGTH=\"" + String(maxlength) + "\">");
  client.println("<INPUT TYPE=SUBMIT NAME=\"submit\" VALUE=\"Save\">");
  client.println("</FORM>");
}              

String findSubmit(String text) {
  int start = header.indexOf("?" + text + "=");
  int end = header.indexOf("&submit=");
  return header.substring(start + text.length() + 2, end);
}

void web_createPage(WiFiClient client) {
  // Display the HTML web page
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  //refresh page every 5 seconds:
  //client.print("<meta http-equiv=\"refresh\" content=\"5\">");
  //
  //client.println("<meta name=\"HandheldFriendly\" content=\"True\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons 
  // Feel free to change the background-color and font-size attributes to fit your preferences
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #77878A;}</style></head>");
}
  