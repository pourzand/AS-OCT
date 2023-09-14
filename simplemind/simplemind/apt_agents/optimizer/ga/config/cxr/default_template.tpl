<!DOCTYPE html>
<html>

<head>
<style>
table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
	padding: 10px;

}
}

.equ {

	border: 0px;
	padding: 10px;
	vertical-align: top;
}
th {
  text-align: left;
}

table#t01 {
	width:100%;
}

table#t01 tr:nth-child(even) {
  background-color: #eee;
}
table#t01 tr:nth-child(odd) {
 background-color: #fff;
}

table#t01 th {
 background-color: #fff;
}


table#t02 tr:nth-child(even) {
  background-color: #fff;
}
table#t02 tr:nth-child(odd) {
 background-color: #fff;
}

</style>
</head>

<body>

<h1>
{{ gene }} <br>
{{ gene_binary }}
</h1>

<h3>
Fitness: {{ final_result["fitness"] }}
</h3>

<img width=100% src="{{ to_html(final_result["error_hist_plot"])}}" >


<h1>
Overall Performance Summary
</h1>

<table class="equ" id="t01">
<!-- 	<col width="200">
	<col width="175">
	<col width="175">
	<col width="175">
	<col width="175"> -->
	<tr class="equ">
		<th class="equ"> </th>
		<th class="equ">Carina</th>
		<th class="equ">GE Junction</th>
		<th class="equ">ETT Tip</th>
		<th class="equ">NG Tube</th>
	</tr>

	<tr class="equ">
		<th class="equ">Mean total error [mm]:</th>
		<td class="equ">{{ final_result["Crina"]["total_error_mean"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["total_error_mean"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["total_error_mean"] }}</td>
		<td class="equ">{{ final_result["NgTub"]["total_error_mean"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Mean x error [mm]:</th>
		<td class="equ">{{ final_result["Crina"]["x_error_mean"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["x_error_mean"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["x_error_mean"] }}</td>
		<td class="equ"> --- </td>
	</tr>
	<tr class="equ">
		<th class="equ">Mean y error [mm]:</th>
		<td class="equ">{{ final_result["Crina"]["y_error_mean"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["y_error_mean"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["y_error_mean"] }}</td>
		<td class="equ"> --- </td>
	</tr>
	<tr class="equ">
		<th class="equ">Number annotated cases:</th>
		<td class="equ">{{ final_result["Crina"]["num_annotated"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["num_annotated"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["num_annotated"] }}</td>
		<td class="equ">{{ final_result["NgTub"]["num_annotated"] }}</td>
	</tr>
		<tr class="equ">
		<th class="equ">Number good predictions:</th>
		<td class="equ">{{ final_result["Crina"]["num_good_markings"] }} cases with less than {{ final_result["Crina"]["threshold"] }} mm error</td>
		<td class="equ">{{ final_result["GEjct"]["num_good_markings"] }} cases with less than {{ final_result["GEjct"]["threshold"] }} mm error</td>
		<td class="equ">{{ final_result["EtTip"]["num_good_markings"] }} cases with less than {{ final_result["EtTip"]["threshold"] }} mm error</td>
		<td class="equ">{{ final_result["NgTub"]["num_good_markings"] }} cases with less than {{ final_result["NgTub"]["threshold"] }} mm error</td>
	</tr>
	<!-- <tr class="equ"> -->
		<!-- <td class="equ">Bad markings:</td> -->
		<!-- <td class="equ">{{ final_result["Crina"]["num_bad_markings"] }}</td> -->
		<!-- <td class="equ">{{ final_result["GEjct"]["num_bad_markings"] }}</td> -->
		<!-- <td class="equ">{{ final_result["EtTip"]["num_bad_markings"] }}</td> -->
		<!-- <td class="equ">{{ final_result["NgTub"]["num_bad_markings"] }}</td> -->
	<!-- </tr> -->
	<tr class="equ">
		<th class="equ">Detection:</th>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["Crina"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["Crina"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["Crina"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["Crina"]["tn"] }}</td>
				</tr>
			</table>

		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["GEjct"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["GEjct"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["GEjct"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["GEjct"]["tn"] }}</td>
				</tr>
			</table>
		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["EtTip"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["EtTip"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["EtTip"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["EtTip"]["tn"] }}</td>
				</tr>
			</table>
		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["NgTub"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["NgTub"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["NgTub"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["NgTub"]["tn"] }}</td>
				</tr>
			</table>
		</td>
	</tr>

	<tr class="equ">
		<th class="equ">Sensitivity:</th>
		<td class="equ">{{ final_result["Crina"]["sensitivity"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["sensitivity"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["sensitivity"] }}</td>
		<td class="equ">{{ final_result["NgTub"]["sensitivity"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Specificity:</th>
		<td class="equ">{{ final_result["Crina"]["specificity"] }}</td>
		<td class="equ">{{ final_result["GEjct"]["specificity"] }}</td>
		<td class="equ">{{ final_result["EtTip"]["specificity"] }}</td>
		<td class="equ">{{ final_result["NgTub"]["specificity"] }}</td>
	</tr>

</table>




{% if final_result["alert"] -%}
<h1>High error cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["alert"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


{% if final_result["misplaced"] -%}
<h1>Incorrect placement cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["misplaced"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


{% if final_result["other"] -%}
<h1>Other cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["other"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>


{% else -%}
<h1>All cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in keys -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


</body>

</html>