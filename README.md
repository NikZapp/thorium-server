# Thorium
Thorium is an easy verification system made with c++. It can be integrated with other programs using json files.
# Setup
Compile the program by running `cmake --build . --target all`  
The binary should be located at `build/default/thorium`

Ensure that `tokens.json` and `authorized.json` are available in the working directory and the port 22542 is open for TCP connections.
# How to verify
## Step 1: Creating a token
A token is usually a sha256 hashed password, or a random byte sequence. The token should not be shared. To generate a token from a password, this python one-liner can be used:
```python
from hashlib import sha256; print(sha256(input("Enter password:").encode()).digest().hex())
```
You will be prompted a password, and a token will be generated from it.
## Step 2: Getting a token on the server
If you do not have access to the verification server, you will need to ask an administrator to add your token. However, if you are an admin, you can add a token by modifying `tokens.json`. After modifying the file, the authentication system should be restarted.

An example configuration file looks like this:
```json
{
	"Alice": {
		"token": "6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b",
		"level": 2
	},
	"Bob" : {
		"token": "d4735e3a265e16eee03f59718b9b5d03019c07d8b6c51f90da3a666eec13ab35",
		"level": 3
	},
	"Carol": {
		"token": "4e07408562bedb8b60ce05c1decfe3ad16b72230967de01f640b7e4729b49fce",
		"level": 2
	},
	"Dave": {
		"token": "4b227777d4dd1fc61c6f884f48641d02b4d121d3fd328cb08b5531fcacdabf8a",
		"level": 1
	},
	"Eve": {
		"token": "ef2d127de37b942baad06145e54b0c619a1f22327b2ebbcfbec78f5564afe39d",
		"level": 0
	}
}
```
Note: These tokens correspond to passwords "1", "2", "3", "4" and "5"

Every key in the list represents the token name, which should be unique per person.

The `token` field stores the generated token in hex format, while the `level` field stores the permissions/access level for a given token. In this example, `Bob` has the highest level of 3, while `Eve` has the lowest level of 0.

When a user asks you to add their token, the `level` should be assigned according to their permissions.
## Step 3: Verifying the token
An official implementation for a verifier written in python is available [here](https://cdn.discordapp.com/attachments/602689326039433236/1183381683274272949/thorium_client.py), however, any verifier can be used as long as it follows the protocol implementation.

If the token has matched, it is added to a file called `authorized.json`, which looks something like this:
```json
{
	"123.45.67.89": {
		"level": 3,
		"name": "Bob"
	},
	"234.56.78.90": {
		"level": 2,
		"name": "Alice"
	}
}
```
# Protocol implementation
A client connects using TCP to port 22542, and the server immediately sends a string of random bytes preceded by "K". In response, the client should send the sha256 hash of the combination of the token and the received bytes. The server later checks if the resulting "proof" matches with one, generated using the stored tokens. The server responds with one of the following:

| First char | Meaning                                                  |
| ---------- | -------------------------------------------------------- |
| P          | The client has passed the test and has been verified     | 
| F          | The client failed to pass the test and has been rejected |
| T          | The server waited too long for a proof.                  |

Anything following the initial character can be used to explain the reason for the verdict, however that is optional.

If, after being successfully verified and within 5 seconds, a client sends a 4 byte response containing the encoded ip, that ip will be used instead of the connection ip. This allows for third party verification clients to report the actual user ip.
# Used libraries
picosha for hashing functions: https://github.com/okdshin/PicoSHA2 <br>
nlohmann::json for the json library: https://github.com/nlohmann/json

