## Keystroke attack Side Channel

This directory contains code for our Keystroke attack side-channel.

- `client.py` is code for the remote client
- `server` contains the code for the server
- `prober` contains the code for the server side malicious prober
- `download-dataset.sh` and `gen_ground_truth.py` download and generate the inter-keystroke timing the client will use
- `gen_stats.py` generates the final accuracy number from the prober and the ground truth
