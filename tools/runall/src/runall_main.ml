(*
 * (C) Copyright IBM Corp. 2019.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *)

open Printf

let stdin = open_in "/dev/stdin"
let opt_o = ref "unspecified"
let opt_o_channel = ref stdout
let opt_parse_only = ref false
let opt_verbose = ref false
let opt_fmt_in = ref "unspecified"
let opt_fmt_out = ref "unspecified"
let opt_exec = ref false

let synopsis prog =
  printf "%s\n" (Filename.basename prog);
  printf "usage: %s <option>* <infile>?\n" (Filename.basename prog);
  List.iter (output_string stdout)
    ["options:\n";
     "  -p\t\t\tterminate after parsing\n";
     "  -o <file>\t\toutput generated script to <file>\n";
     "  -t <fmt>\t\toutput in <fmt> (\"yojson\", \"caml\")\n";
     "  --exec\t\texecute generated script using bash (** be careful)\n";
     "  -h\t\t\tdisplay this message\n"]

(* *)
let main argc argv =
  let i = ref 1
  and infile = ref "/dev/stdin"
  and outfile = ref "/dev/stdout" in
  while !i < argc do
    let _ =
      match argv.(!i) with
      | "-" ->
	  infile := "/dev/stdin";
      | "-o" | "--output" when !i + 1 < argc ->
	  outfile := argv.(!i+1); incr i;
      | "-t" when !i + 1 < argc ->
	  opt_fmt_out := argv.(!i+1); incr i;

      | "-p" | "--parse-only" ->
	  opt_parse_only := true
      | "--exec" ->
	  opt_exec := true
      | "--no-exec" | "--scriptgen" ->
	  opt_exec := false

      | "-v" | "--verbose" ->
	  opt_verbose := true
      | "-q" | "--silent" ->
	  opt_verbose := false
      | "-h" | "--help"  ->
	  synopsis argv.(0); exit 0

      | _ when argv.(!i).[0] = '-' ->
	  invalid_arg @@ "invalid option: " ^ argv.(!i)
      | _    ->
	  infile := argv.(!i)
    in incr i
  done;

  (* input/output channels *)
  if not (Sys.file_exists !infile) then
    (eprintf "file does not exist: %S\n" !infile; raise Not_found);
  let ic = open_in !infile and oc = open_out !outfile
  in at_exit (fun _ -> close_out oc);

  (* translation *)
  let p : Process.t = Process.from_channel ic in
  close_in ic;
  if !opt_parse_only then
    begin
      Process.to_channel ~format: !opt_fmt_out oc p;
      output_string oc "\n";
      raise Exit
    end;

  let p = Scripter.canonicalize p in

  (* output *)
  if not !opt_exec then
    begin
      let _ =
	match !opt_fmt_out with
	| "script" | "unspecified" ->
	    let tm = Unix.time () |> Unix.gmtime in
	    Printf.fprintf oc "# generated from %S by %s" !infile (Filename.basename argv.(0));
	    Printf.fprintf oc " at %d-%02d-%02dT%02d:%02d:%02d\n"
	      (tm.tm_year + 1900) (tm.tm_mon + 1) tm.tm_mday tm.tm_hour tm.tm_min tm.tm_sec;
	    Printf.fprintf oc "# (https://github.com/ldltools/scxmlrun/tree/master/tools/)\n";
	    Scripter.scriptize ~verbose: !opt_verbose oc p
	| _ ->
	    Process.to_channel ~format: !opt_fmt_out oc p;
	    output_string oc "\n"
      in
      flush_all (); raise Exit
    end;

  (* exec script -- potentially dangerous *)
  failwith "\"--exec\" is currently not supported";
  let temp_name, temp_ch = Filename.open_temp_file "runall" ".sh"
  in
  at_exit (fun _ -> Sys.remove temp_name);
  Scripter.scriptize ~verbose: !opt_verbose temp_ch p;
  close_out temp_ch;
  match Unix.system ("/bin/bash " ^ temp_name) with
  | Unix.WEXITED code -> exit code
  | _ -> exit 1
;;

(* toplevel *)
assert (not !Sys.interactive);
try
  main (Array.length Sys.argv) Sys.argv
with   
| Exit -> exit 0
| Failure s ->
    flush_all ();
    eprintf ";; Failure: %s\n" s;
    exit 1;
| Not_found ->
    flush_all ();
    eprintf ";; Something seems missing!\n";
    exit 1;
| End_of_file ->
    flush_all ();
    eprintf ";; Unexpected end of file\n";
    exit 1;
