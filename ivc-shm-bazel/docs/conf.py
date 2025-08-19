import os
import sys
sys.path.insert(0, os.path.abspath('../..'))  # Make repo root visible

extensions = [
    "sphinx_needs",
    "sphinx_design",
    "sphinx_copybutton",
    "sphinxcontrib.mermaid",
    "sphinx_tabs.tabs",
    "myst_parser"
]

needs_types = [
    dict(directive="req", title="Requirement", prefix="REQ_", color="#BFD8D2", style="node"),
    dict(directive="spec", title="Specification", prefix="SPEC_", color="#FEDCD2", style="node"),
    dict(directive="test", title="Test Case", prefix="TEST_", color="#DF744A", style="node"),
]

needs_id_required = True
needs_id_regex = r"[A-Z_0-9]+"

templates_path = ['_templates']
exclude_patterns = []
html_theme = 'alabaster'
