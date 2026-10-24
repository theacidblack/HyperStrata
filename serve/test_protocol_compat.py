"""serve/test_protocol_compat.py - mock-engine protocol fixture suite (no GPU, no pack, no downloads).

Proves each claimed route honors its request/response contract over real HTTP
against serve() with MockEngine. Harness duplicated minimally from
serve/test_server.py (RecordingEngine/post pattern): ByteTokenizer + Service +
ChatTemplate + serve() on 127.0.0.1, urllib round-trips only.

    python -m pytest serve/test_protocol_compat.py -q
"""
from __future__ import annotations

import http.client
import json
import sys
import unittest
import urllib.error
import urllib.request
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from serve.frontend import ChatTemplate  # noqa: E402
from serve.server import ByteTokenizer, MockEngine, Service, serve  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
TEMPLATE = ChatTemplate(ROOT / "serve/chat_template.jinja")
CTX = 4096
ANSWER = "</think>\n\nhello world"
CALL = ("</think>\n\nLet me check.\n\n<tool_call>\n<function=get_weather>\n"
        "<parameter=city>\nParis\n</parameter>\n</function>\n</tool_call>")
TOOLS_OAI = [{"type": "function", "function": {
    "name": "get_weather", "description": "weather",
    "parameters": {"type": "object", "properties": {"city": {"type": "string"}},
                   "required": ["city"]}}}]
TOOLS_ANTH = [{"name": "get_weather", "description": "weather",
               "input_schema": {"type": "object",
                                "properties": {"city": {"type": "string"}}}}]


def _start(script, ctx=CTX):
    tok = ByteTokenizer()
    svc = Service(MockEngine(tok, script, max_context=ctx), tok, TEMPLATE)
    httpd = serve(svc, port=0)
    return svc, httpd, f"http://127.0.0.1:{httpd.server_address[1]}"


def _post(base, path, body):
    req = urllib.request.Request(base + path, data=json.dumps(body).encode(),
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            return r.status, dict(r.headers), json.loads(r.read())
    except urllib.error.HTTPError as e:
        with e:
            return e.code, dict(e.headers), json.loads(e.read())


def _post_text(base, path, body):
    req = urllib.request.Request(base + path, data=json.dumps(body).encode(),
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            return r.status, dict(r.headers), r.read().decode()
    except urllib.error.HTTPError as e:
        with e:
            return e.code, dict(e.headers), e.read().decode()


def _get(base, path):
    try:
        with urllib.request.urlopen(base + path, timeout=30) as r:
            return r.status, json.loads(r.read())
    except urllib.error.HTTPError as e:
        with e:
            return e.code, json.loads(e.read())


def _raw(base, path, raw: bytes):
    c = http.client.HTTPConnection("127.0.0.1", int(base.rsplit(":", 1)[1]), timeout=30)
    try:
        c.request("POST", path, body=raw, headers={"Content-Type": "application/json"})
        r = c.getresponse()
        return r.status, json.loads(r.read())
    finally:
        c.close()


def _sse_chunks(text):
    chunks, done = [], False
    for line in text.splitlines():
        if line == "data: [DONE]":
            done = True
        elif line.startswith("data: {"):
            chunks.append(json.loads(line[6:]))
    return chunks, done


class PlainProtocol(unittest.TestCase):
    """Plain-text script: non-stream, stream, length, anthropic, responses, errors, models, alias."""

    @classmethod
    def setUpClass(cls):
        cls.svc, cls.httpd, cls.base = _start(ANSWER)

    @classmethod
    def tearDownClass(cls):
        cls.httpd.shutdown()
        cls.httpd.server_close()

    def test_01_openai_non_stream(self):
        s, _, b = _post(self.base, "/v1/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}]})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["object"], "chat.completion")
        self.assertEqual(b["choices"][0]["message"]["role"], "assistant")
        self.assertEqual(b["choices"][0]["finish_reason"], "stop")
        for k in ("prompt_tokens", "completion_tokens", "total_tokens"):
            self.assertIn(k, b["usage"])

    def test_02_openai_stream(self):
        _, _, ref = _post(self.base, "/v1/chat/completions",
                          {"model": "m", "messages": [{"role": "user", "content": "hi"}]})
        s, headers, text = _post_text(self.base, "/v1/chat/completions",
                                     {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                                      "stream": True})
        self.assertEqual(s, 200)
        self.assertIn("text/event-stream", headers.get("Content-Type", ""))
        chunks, done = _sse_chunks(text)
        self.assertTrue(done)
        self.assertEqual(chunks[0]["choices"][0]["delta"].get("role"), "assistant")
        content = "".join(c["choices"][0]["delta"].get("content", "") for c in chunks)
        self.assertEqual(content, ref["choices"][0]["message"]["content"])
        self.assertIn("usage", chunks[-1])

    def test_05_length(self):
        s, _, b = _post(self.base, "/v1/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                         "max_tokens": 2})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["choices"][0]["finish_reason"], "length")

    def test_06_anthropic_plain(self):
        s, _, b = _post(self.base, "/v1/messages",
                        {"model": "m", "max_tokens": 64,
                         "messages": [{"role": "user", "content": "hi"}]})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["stop_reason"], "end_turn")
        self.assertTrue(b["content"])
        for block in b["content"]:
            self.assertIn(block["type"], ("text", "tool_use", "thinking", "redacted_thinking"))

    def test_07_responses(self):
        s, _, b = _post(self.base, "/v1/responses", {"model": "m", "input": "hi"})
        self.assertEqual(s, 200, b)
        self.assertIsInstance(b.get("output"), list)
        s2, b2 = _get(self.base, "/v1/responses/does-not-exist")
        self.assertEqual(s2, 404)
        self.assertIn("error", b2)

    def test_08_errors(self):
        s, b = _raw(self.base, "/v1/chat/completions", b"{not json")
        self.assertEqual(s, 400)
        self.assertEqual(b["error"]["type"], "invalid_request_error")
        s, _, b = _post(self.base, "/v1/nope", {"model": "m"})
        self.assertEqual(s, 404)
        self.assertEqual(b, {"error": {"message": "not found"}})
        with mock.patch.object(self.svc, "encode_prompt", return_value=[ord("x")] * (CTX + 100)):
            s, _, b = _post(self.base, "/v1/chat/completions",
                            {"model": "m", "messages": [{"role": "user", "content": "hi"}]})
        self.assertEqual(s, 400, b)
        self.assertEqual(b["error"]["code"], "context_length_exceeded")
        s, _, b = _post(self.base, "/v1/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                         "tools": TOOLS_OAI, "tool_choice": {"type": "bogus"}})
        self.assertEqual(s, 400)
        self.assertIn("bogus", b["error"]["message"])

    def test_09_models_health(self):
        s, b = _get(self.base, "/v1/models")
        self.assertEqual(s, 200)
        self.assertEqual(b["object"], "list")
        self.assertTrue(b["data"][0]["id"])
        s, b = _get(self.base, "/health")
        self.assertEqual(s, 200)
        self.assertEqual(b["status"], "ok")

    def test_10_alias(self):
        _, _, a = _post(self.base, "/v1/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}]})
        s, _, b = _post(self.base, "/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}]})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["object"], "chat.completion")
        self.assertEqual(b["choices"][0]["message"]["content"],
                         a["choices"][0]["message"]["content"])


class ToolProtocol(unittest.TestCase):
    """Tool-call script: OpenAI non-stream/stream fragment merge + Anthropic tool_use."""

    @classmethod
    def setUpClass(cls):
        cls.svc, cls.httpd, cls.base = _start(CALL)

    @classmethod
    def tearDownClass(cls):
        cls.httpd.shutdown()
        cls.httpd.server_close()

    def test_03_tools_non_stream(self):
        s, _, b = _post(self.base, "/v1/chat/completions",
                        {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                         "tools": TOOLS_OAI, "tool_choice": "auto"})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["choices"][0]["finish_reason"], "tool_calls")
        fn = b["choices"][0]["message"]["tool_calls"][0]["function"]
        self.assertEqual(fn["name"], "get_weather")
        self.assertEqual(json.loads(fn["arguments"]), {"city": "Paris"})

    def test_04_tools_stream(self):
        _, _, ref = _post(self.base, "/v1/chat/completions",
                          {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                           "tools": TOOLS_OAI, "tool_choice": "auto"})
        want_args = ref["choices"][0]["message"]["tool_calls"][0]["function"]["arguments"]
        s, _, text = _post_text(self.base, "/v1/chat/completions",
                                {"model": "m", "messages": [{"role": "user", "content": "hi"}],
                                 "tools": TOOLS_OAI, "tool_choice": "auto", "stream": True})
        self.assertEqual(s, 200)
        chunks, done = _sse_chunks(text)
        self.assertTrue(done)
        merged: dict[int, dict] = {}
        for c in chunks:
            for tc in c["choices"][0]["delta"].get("tool_calls") or []:
                m = merged.setdefault(tc.get("index", 0), {"args": ""})
                fn = tc.get("function", {})
                if tc.get("id"):
                    m["id"] = tc["id"]
                if fn.get("name"):
                    m["name"] = fn["name"]
                m["args"] += fn.get("arguments", "")
        self.assertEqual(len(merged), 1)
        only = next(iter(merged.values()))
        self.assertEqual(only["name"], "get_weather")
        self.assertEqual(json.loads(only["args"]), json.loads(want_args))
        self.assertEqual(chunks[-1]["choices"][0]["finish_reason"], "tool_calls")

    def test_06_anthropic_tool(self):
        s, _, b = _post(self.base, "/v1/messages",
                        {"model": "m", "max_tokens": 256,
                         "messages": [{"role": "user", "content": "hi"}],
                         "tools": TOOLS_ANTH})
        self.assertEqual(s, 200, b)
        self.assertEqual(b["stop_reason"], "tool_use")
        uses = [x for x in b["content"] if x["type"] == "tool_use"]
        self.assertTrue(uses)
        self.assertEqual(uses[0]["name"], "get_weather")
        self.assertEqual(uses[0]["input"], {"city": "Paris"})


if __name__ == "__main__":
    unittest.main()
