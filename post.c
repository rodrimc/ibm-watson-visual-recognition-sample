#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libsoup/soup.h>


static SoupSession *session;
static GMainLoop *loop;
static const gchar *output_file_path = NULL;
static gboolean head, quiet;

static void
finished (SoupSession *session, SoupMessage *msg, gpointer loop)
{
	g_main_loop_quit (loop);
}

static void
get_url (const char *api_key, const char *filepath)
{
	const char *name;
	SoupMessage *msg;
  gsize length;
  guchar *buffer;
  gchar *uri;
	FILE *output_file = NULL, *file = NULL;
  SoupURI *soupURI = NULL;
  SoupMultipart *multipart = NULL;
  SoupBuffer *soup_buffer = NULL;
  const char *url = 
    "https://gateway-a.watsonplatform.net/visual-recognition/api/v3/classify";

  file = fopen (filepath, "rb");
  if (!file)
  {
    g_printerr ("Could not open %s\n.", filepath);
    return;
  }

  fseek (file, 0L, SEEK_END); 
  length = ftell (file);
  rewind (file);

  buffer = g_malloc (length + 1); 
  fread (buffer, sizeof(guchar), length, file);

  fclose (file);

  soup_buffer = soup_buffer_new_take (buffer, length);
  multipart = soup_multipart_new ("multipart/form-data");
  soup_multipart_append_form_file (multipart, "images_data", filepath, 
      "image/jpg", soup_buffer);

  soupURI = soup_uri_new (url);
  soup_uri_set_query_from_fields (soupURI, 
      "api_key", api_key,
      "version", "2016-05-19",
      NULL);

  uri = soup_uri_to_string (soupURI, FALSE);
	msg = soup_form_request_new_from_multipart (uri, multipart);
  soup_uri_free (soupURI);
  g_free (uri);

	soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);

	if (loop) {
		g_object_ref (msg);
		soup_session_queue_message (session, msg, finished, loop);
		g_main_loop_run (loop);
	} else
		soup_session_send_message (session, msg);

	name = soup_message_get_uri (msg)->path;

  if (msg->status_code == SOUP_STATUS_SSL_FAILED) {
    GTlsCertificateFlags flags;

    if (soup_message_get_https_status (msg, NULL, &flags))
      g_print ("%s: %d %s (0x%x)\n", name, msg->status_code, msg->reason_phrase, flags);
    else
      g_print ("%s: %d %s (no handshake status)\n", name, msg->status_code, msg->reason_phrase);
  } else if (!quiet || SOUP_STATUS_IS_TRANSPORT_ERROR (msg->status_code))
    g_print ("%s: %d %s\n", name, msg->status_code, msg->reason_phrase);

	if (!head && SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		if (output_file_path) {
			output_file = fopen (output_file_path, "w");
			if (!output_file)
				g_printerr ("Error trying to create file %s.\n", output_file_path);
		} else if (!quiet)
			output_file = stdout;

		if (output_file) {
			fwrite (msg->response_body->data,
				1,
				msg->response_body->length,
				output_file);

			if (output_file_path)
				fclose (output_file);
		}
	}
  soup_buffer_free (soup_buffer);
}

int
main (int argc, char **argv)
{
	SoupURI *parsed;
	SoupLogger *logger = NULL;
	SoupMessage *msg;
	const char *api_key, *filepath;

	if (argc < 3) {
		g_printerr ("Usage: %s <api_key> <FILE>\n", argv[0]);
    return EXIT_FAILURE;
	}

	api_key = argv[1];
	filepath = argv[2];

  session = soup_session_new ();

  logger = soup_logger_new (SOUP_LOGGER_LOG_BODY, -1);
  soup_session_add_feature (session, SOUP_SESSION_FEATURE (logger));
  g_object_unref (logger);

  loop = g_main_loop_new (NULL, TRUE);

  get_url (api_key, filepath);
  g_main_loop_unref (loop);
  g_object_unref (session);

  return EXIT_SUCCESS;
}
