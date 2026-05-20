name: Bug report
description: Create a report to help us improve AlphaMon assets
labels: ["bug"]
body:
  - type: markdown
    attributes:
      value: |
        Thanks for taking the time to fill out this bug report!
  - type: textarea
    id: what-happened
    attributes:
      label: What happened?
      description: Also tell us what you expected to happen
      placeholder: Describe the issue in detail...
    validations:
      required: true
  - type: input
    id: version
    attributes:
      label: AlphaMon Firmware Version / PCB Revision
      placeholder: e.g. Firmware v2.1.0, PCB Rev B
  - type: textarea
    id: logs
    attributes:
      label: Logs / Configuration
      description: Please copy and paste any relevant logs or configuration snippets here.
      render: shell

