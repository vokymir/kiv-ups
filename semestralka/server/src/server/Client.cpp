/* TODO: in a moment, now as a reminder
 *
// Process complete messages (e.g., newline-delimited)
size_t pos;
while ((pos = client.read_buffer.find('\n')) != std::string::npos) {
  std::string message = client.read_buffer.substr(0, pos);
  client.read_buffer.erase(0, pos + 1);

  // Parse and handle the message
  try {
    process_message(fd, message);
  } catch (const std::exception &e) {
    client.invalid_msg_count++;
    if (client.invalid_msg_count >= 3) {
      handle_client_disconnect(fd);
      return;
    }
  }
}
*/
