// This file demonstrates the concept of prompt context injection management
// as requested in the user's query for the icarus project.

package main

import (
	"fmt"
	"time"
)

// PromptContext represents the context information for prompt construction
type PromptContext struct {
	turnCount    int
	sessionStart time.Time
	usedTokens   int
	totalTokens  int
	available    int
	threshold    int
}

// NewPromptContext creates a new prompt context with default settings
func NewPromptContext() *PromptContext {
	return &PromptContext{
		turnCount:    0,
		sessionStart: time.Now(),
		threshold:    3, // Default threshold for injecting context usage
	}
}

// IncrementTurn advances the turn counter
func (pc *PromptContext) IncrementTurn() {
	pc.turnCount++
}

// ShouldInjectContextUsage determines if context usage should be injected
func (pc *PromptContext) ShouldInjectContextUsage() bool {
	return pc.turnCount >= pc.threshold
}

// GetContextUsageString returns the context usage string for injection
func (pc *PromptContext) GetContextUsageString() string {
	return fmt.Sprintf("[context] session start: %d of %d tokens used (%.0f%%); %d tokens available; compaction (/cmp --clear) fires at 70%%.",
		pc.usedTokens, pc.totalTokens, float64(pc.usedTokens)/float64(pc.totalTokens)*100, pc.available)
}

// GetVolatileContent returns volatile content that might be injected into stable prefix
func (pc *PromptContext) GetVolatileContent() []string {
	volatile := []string{}

	// Time-based volatile content
	now := time.Now()
	volatile = append(volatile, fmt.Sprintf("time: %02d:%02d", now.Hour(), now.Minute()))

	// Session ID (if any)
	volatile = append(volatile, "session_id: ses_123456")

	// Other potential volatile elements
	volatile = append(volatile, "timestamp: "+now.Format("2006-01-02T15:04:05Z07:00"))

	return volatile
}

// InjectContextIntoPrompt injects context usage at the end of the prompt
func (pc *PromptContext) InjectContextIntoPrompt(prompt string) string {
	if !pc.ShouldInjectContextUsage() {
		return prompt // Don't inject context usage yet
	}

	// Inject context usage at the end of the prompt
	contextUsage := pc.GetContextUsageString()

	// Add context usage to the end of the prompt
	return fmt.Sprintf("%s\n\n[Context Usage Info]\n%s", prompt, contextUsage)
}

// Example usage
func main() {
	ctx := NewPromptContext()

	// Simulate a conversation with context injection
	prompt := "User: What is the current system status?"

	fmt.Println("Initial prompt:", prompt)

	// Simulate first few turns (no context injection)
	for i := 1; i <= 3; i++ {
		ctx.IncrementTurn()
		fmt.Printf("\nTurn %d (no injection yet):\n", i)
		fmt.Println("Prompt:", ctx.InjectContextIntoPrompt(prompt))
	}

	// After threshold, inject context usage
	ctx.IncrementTurn()
	fmt.Printf("\nTurn %d (injecting context):\n", ctx.turnCount)
	fmt.Println("Prompt:", ctx.InjectContextIntoPrompt(prompt))

	// Show volatile content that might be in stable prefix
	fmt.Println("\nVolatile content that might be injected into stable prefix:")
	for _, volatile := range ctx.GetVolatileContent() {
		fmt.Println("  ", volatile)
	}
}
