import subprocess
import sys
from pathlib import Path

SERVICE_NAME = "InterceptKeysService"
DISPLAY_NAME = "InterceptKeys"
DESCRIPTION = "Intercept key strokes to send modified key strokes using interception kernel-mode driver. (https://github.com/IMXEren/InterceptKeys)"
BIN_PATH = Path.cwd().resolve() / "x64/Release/InterceptKeys.exe"


def main():
    add_to_service()


def run_command_in_console(command: list[str]) -> int:
    """Run a command and print to console."""
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,  # Line-buffered
    )
    for line in process.stdout:  # type: ignore
        print(line, end="")
    process.wait()
    return process.returncode


def add_to_service():
    """Add the task to the Task Scheduler."""
    delete_command = ["sc", "delete", SERVICE_NAME]
    create_command = [
        "sc",
        "create",
        SERVICE_NAME,
        "binPath=",
        BIN_PATH,
        "DisplayName=",
        DISPLAY_NAME,
        "start=",
        "auto",
    ]
    query_config_command = ["sc", "qc", SERVICE_NAME]
    query_description_command = ["sc", "qdescription", SERVICE_NAME]
    set_description_command = ["sc", "description", SERVICE_NAME, DESCRIPTION]
    # Set recovery actions:
    #   - restart on 1st, 2nd, and subsequent failures
    #   - 60000ms (1 minute) delay before restart
    #   - reset failure count after 0 days
    recovery_actions_command = [
        "sc",
        "failure",
        SERVICE_NAME,
        "reset=",
        "0",
        "actions=",
        "restart/60000/restart/60000/restart/60000",
    ]
    # Required to enable recovery actions on crash
    enable_recovery_command = ["sc", "failureflag", SERVICE_NAME, "1"]
    run_service_command = ["sc", "start", SERVICE_NAME]

    exit_code = run_command_in_console(create_command)
    if exit_code != 0:
        print(f"Failed to create service {SERVICE_NAME}. Error code: {exit_code}")
        if exit_code == 1073:  # ERROR_SERVICE_EXISTS
            print(
                "----------------------------------------------------------------------------------------\n"
                "                                     SERVICE INFO                                       \n"
                "----------------------------------------------------------------------------------------"
            )
            run_command_in_console(query_description_command)
            run_command_in_console(query_config_command)
            print(
                "----------------------------------------------------------------------------------------\n"
                "                                     xxxxxxxxxxxx                                       \n"
                "----------------------------------------------------------------------------------------"
            )
            user_input = (
                input("Do you want to update the service? [y/n] ").strip().lower()
            )
            if user_input != "y":
                return
    else:
        print(f"Service {SERVICE_NAME} created successfully.")

    exit_code = run_command_in_console(set_description_command)
    if exit_code != 0:
        print(
            f"Failed to set description for service {SERVICE_NAME}. Error code: {exit_code}"
        )
        return

    exit_code = run_command_in_console(enable_recovery_command)
    if exit_code != 0:
        print(
            f"Failed to enable recovery from failure for service {SERVICE_NAME}. Error code: {exit_code}"
        )
        return

    exit_code = run_command_in_console(recovery_actions_command)
    if exit_code != 0:
        print(
            f"Failed to set recovery actions for service {SERVICE_NAME}. Error code: {exit_code}"
        )
        return
    print(f"Recovery from failure, enabled for service {SERVICE_NAME}.")

    exit_code = run_command_in_console(run_service_command)
    if exit_code != 0:
        print(f"Failed to start service {SERVICE_NAME}. Error code: {exit_code}")
        return
    print(f"Service {SERVICE_NAME} started successfully.")


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        BIN_PATH = Path(sys.argv[1]).resolve()
        print(f"Using executable path: {BIN_PATH}")
    else:
        print(
            "No executable path provided. Using default: x64/Release/InterceptKeys.exe"
        )
    main()
