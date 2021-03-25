<?php


$json = <<<'JSON'
{
    "update_id": 346089057,
    "message": {
        "message_id": 525,
        "from": {
            "id": 243692601,
            "is_bot": false,
            "first_name": "cpu_relax();",
            "last_name": "//",
            "username": "ammarfaizi2",
            "language_code": "en"
        },
        "chat": {
            "id": -1001422514298,
            "title": "GNU/Weeb Test Driven Development",
            "username": "GNUWeebTDD",
            "type": "supergroup"
        },
        "date": 1616588789,
        "text": "/debug",
        "entities": [
            {
                "offset": 0,
                "length": 6,
                "type": "bot_command"
            }
        ],
        "reply_to_message": null
    }
}
JSON; /* end JSON */


function main(): int
{
	global $json;

	$sock = socket_create(AF_INET, SOCK_STREAM, 0);
	$conn = socket_connect($sock, "127.0.0.1", 55555);

	if (!$conn)
		return 1;

	$payload     = pack("n", strlen($json)).$json;
	$write_len   = socket_write($sock, $payload);
	$correct_len = strlen($payload);

	if ($write_len != $correct_len) {
		printf("socket_write failed: write_len = %d; correct_len = %d\n",
		       $write_len, $correct_len);
		return 1;
	}

	socket_close($sock);
	return 0;
}

exit(main());
