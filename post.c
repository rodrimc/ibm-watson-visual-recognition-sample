#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libsoup/soup.h>


static SoupSession *session;
static GMainLoop *loop;
static const char *url = 
    "https://gateway-a.watsonplatform.net/visual-recognition/api/v3/classify";

static void
finished (SoupSession *session, SoupMessage *msg, gpointer loop)
{
	g_main_loop_quit (loop);
}

static void
call_service (const char *api_key, const char *filepath)
{
	const char *name;
	SoupMessage *msg;
  gsize length;
  guchar *buffer;
  gchar *uri;
	FILE *file = NULL;
  SoupURI *soupURI = NULL;
  SoupMultipart *multipart = NULL;
  SoupBuffer *soup_buffer = NULL;

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

	soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);

  g_object_ref (msg);
  soup_session_queue_message (session, msg, finished, loop);
  g_main_loop_run (loop);

  soup_uri_free (soupURI);
  soup_buffer_free (soup_buffer);
  soup_multipart_free (multipart);
  g_free (uri);

	name = soup_message_get_uri (msg)->path;

  if (SOUP_STATUS_IS_TRANSPORT_ERROR (msg->status_code))
    g_print ("%s: %d %s\n", name, msg->status_code, msg->reason_phrase);

	if (SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) 
    fprintf (stdout, "%s", msg->response_body->data);

  g_object_unref (msg);
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

  call_service (api_key, filepath);
  g_main_loop_unref (loop);
  g_object_unref (session);

  return EXIT_SUCCESS;
}
