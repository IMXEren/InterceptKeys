import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

input_xml = "Run_InterceptKeys.xml.example"
output_xml = input_xml.replace(".xml.example", ".xml")
task_name = "\\InterceptKeys\\Run_InterceptKeys"


def main():
    modify_task_xml(input_xml, output_xml)
    add_to_task_scheduler(task_name, output_xml)


def modify_task_xml(input_xml, output_xml):
    """Modify the XML file to set the necessary changes."""
    tree = ET.parse(input_xml)
    root = tree.getroot()

    # Namespace mapping
    ns = {"ns": "http://schemas.microsoft.com/windows/2004/02/mit/task"}
    command_elem = root.find(".//ns:Command", ns)
    if command_elem is not None:
        runner_path = Path.cwd().resolve() / "x64/Release/InterceptKeys.exe"
        command_elem.text = str(runner_path)

        # Register the default namespace to avoid 'ns0'
        ET.register_namespace(
            "", "http://schemas.microsoft.com/windows/2004/02/mit/task"
        )
        tree.write(output_xml, encoding="utf-16", xml_declaration=True)


def add_to_task_scheduler(task_name, task_xml):
    """Add the task to the Task Scheduler."""
    delete_command = ["schtasks.exe", "/delete", "/tn", f"{task_name}", "/f"]
    create_command = [
        "schtasks.exe",
        "/create",
        "/tn",
        f"{task_name}",
        "/xml",
        f"{task_xml}",
    ]
    run_command = ["schtasks.exe", "/run", "/TN", f"{task_name}"]

    process = subprocess.Popen(
        delete_command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,  # Line-buffered
    )
    for line in process.stdout:  # type: ignore
        print(line, end="")
    process.wait()
    if process.returncode == 0:
        print(f"Task {task_name} deleted successfully.")
    else:
        print(f"Failed to delete task {task_name}. Error code: {process.returncode}")

    process = subprocess.Popen(
        create_command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,  # Line-buffered
    )
    for line in process.stdout:  # type: ignore
        print(line)
    process.wait()
    if process.returncode == 0:
        print(f"Task {task_name} created successfully.")

        process = subprocess.Popen(
            run_command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,  # Line-buffered
        )
        for line in process.stdout:  # type: ignore
            print(line)
        process.wait()
        if process.returncode == 0:
            print(f"Task {task_name} ran successfully.")
        else:
            print(f"Failed to run task {task_name}. Error code: {process.returncode}")

    else:
        print(f"Failed to create task {task_name}. Error code: {process.returncode}")


if __name__ == "__main__":
    main()
