create a new branch named agentic and check it out

# Feature Request: dwm-agentic

## Summary

i want to embedd the local model that is running icarus (laguna) in dwm. the idea is that the mode will have the ability to manage window clients, and everything running in the x_session will be part of the context for the model. the model should be invoked via 'super+o' which will open a floating window on the right that is a user prompt and chat window with the model.

we will need to create a set of skills specific for this model to manage window clients, open programs, run scripts that manage the windows.

this model should also be able to communicate with the embedded model in the web-ui 'icarus'. both of these combined should work in tandem when there are prompts from the user at the x-server level about managing tabs, and multiple browser window clients.

the floating window should persist between all tags when called, but should also be togglable (where it can also be closed)

## Workflow

<!-- Describe the step-by-step user experience as numbered actions -->

1.

## Data Types

<!-- Declare input/output data types so the model selects correct algorithms and structures -->

**Inputs (data the model will work with):**

- e.g. lines as `[]string`, a JSON payload, `map[string]T`, file paths

**Outputs (data the model must produce):**

- e.g. a typed struct/DTO, a sorted slice, an exit code, a written file

## Implementation Details

<!-- Technical breakdown: file paths, code changes, config, integration points -->

###

## Considerations

<!-- Edge cases, trade-offs, alternatives, dependencies, open questions -->

-

<!-- # Outputs -->
<!-- List concrete deliverable files -->

