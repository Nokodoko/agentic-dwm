package llm

import (
	"context"
	"fmt"
	"sort"
	"strings"
)

// Registry is the set of backends /model can switch between.
type Registry struct {
	backends map[string]Backend
	order    []string
}

// DefaultRegistry is the built-in backend set.
//
// The default is the strongest model reachable from this host, and the
// offline fallback is a genuinely different, smaller model on lewis -- not
// the same weights relocated. GLM lived on monty and was unreachable
// off-network; it is retired here in favour of the lewis R1 server, which
// autostarts on demand and is therefore the default that actually answers.
func DefaultRegistry() *Registry {
	r := &Registry{backends: map[string]Backend{}}
	r.Add(Backend{
		Name:    "deepseek",
		BaseURL: "http://127.0.0.1:8082/v1",
		Model:   "deepseek-r1-distill-qwen:14b",
		Note:    "DeepSeek-R1-Distill-Qwen 14B (llama.cpp on lewis) — default, autostarts",
	})
	r.Add(Backend{
		Name:    "qwen",
		BaseURL: "http://192.168.50.190:30000/v1",
		Model:   "qwen3.8-27b",
		Note:    "Qwen3.8-27B (sglang on monty) — RETIRED 2026-08-27, kept for reference",
	})
	r.Add(Backend{
		Name:    "local",
		BaseURL: "http://127.0.0.1:8083/v1",
		Model:   "qwen3-30b-a3b:moe",
		Note:    "Qwen3-30B-A3B MoE (llama.cpp on lewis) — offline fallback",
	})
	return r
}

// Add registers a backend, preserving insertion order for listings.
func (r *Registry) Add(b Backend) {
	if _, seen := r.backends[b.Name]; !seen {
		r.order = append(r.order, b.Name)
	}
	r.backends[b.Name] = b
}

// Get looks up a backend by name.
func (r *Registry) Get(name string) (Backend, bool) {
	b, ok := r.backends[strings.TrimSpace(name)]
	return b, ok
}

// Names lists backend handles in registration order.
func (r *Registry) Names() []string {
	out := make([]string, len(r.order))
	copy(out, r.order)
	return out
}

// List renders the backends for the /model command with reachability marks.
func (r *Registry) List(ctx context.Context, active string) string {
	var sb strings.Builder
	for _, name := range r.order {
		b := r.backends[name]
		mark := " "
		if name == active {
			mark = "*"
		}
		status := "up"
		if err := New(b).Probe(ctx); err != nil {
			status = "down"
		}
		fmt.Fprintf(&sb, "%s %-8s %-5s %s\n", mark, name, status, b.Note)
	}
	return sb.String()
}

// FirstReachable returns the first backend in preference order that answers,
// so startup can fall back automatically when monty is off-network.
func (r *Registry) FirstReachable(ctx context.Context, preferred ...string) (Backend, error) {
	candidates := preferred
	if len(candidates) == 0 {
		candidates = r.Names()
	}
	var tried []string
	for _, name := range candidates {
		b, ok := r.backends[name]
		if !ok {
			continue
		}
		if err := New(b).Probe(ctx); err == nil {
			return b, nil
		}
		tried = append(tried, name)
	}
	sort.Strings(tried)
	return Backend{}, fmt.Errorf("no reachable backend (tried %s)", strings.Join(tried, ", "))
}
