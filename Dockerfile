# https://docs.github.com/en/actions/sharing-automations/creating-actions/dockerfile-support-for-github-actions
From ubuntu:24.04

COPY entrypoint.sh /entrypoint.sh

# Executes `entrypoint.sh` when the Docker container starts up
ENTRYPOINT ["/entrypoint.sh"]
